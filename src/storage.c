#include <stdio.h>
#include <string.h>
#include "CH58x_common.h"
#include "storage.h"
#include "utils.h"
#include "worktime.h"

#define TAG "ST"

#define ST_ITEM_SIZE(pitem)	(((pitem)->header.len) + sizeof(st_item_header_t))

typedef struct st_item_location{
	uint16_t page;
	uint16_t offset;
}st_item_location_t;

typedef struct st_search_context {
	uint16_t item_idx;
	st_item_header_t last_header;
	st_item_location_t last_location;
} st_search_ctx_t;

typedef struct st_context {
	st_header_t header;
	uint8_t flag_init;
	uint16_t page_erased;
	uint16_t page_active;
	uint16_t page_start;
	uint16_t page_cnt;
	st_page_t pages[ST_PAGE_MAX];
	st_search_ctx_t search_ctx;
} st_ctx_t;
static st_ctx_t st_ctx;

static int st_is_init(){
	return st_ctx.flag_init;
}

static const char *st_page_status_str(uint8_t status){
	switch(status){
		case ST_PAGE_S_ERASED:
			return "ERASED";
		case ST_PAGE_S_ACTIVE:
			return "ACTIVE";
		case ST_PAGE_S_FULL:
			return "FULL";
		case ST_PAGE_S_UNAVAILABLE:
			return "UNAVAIABLE";
		default:
			return "UNKNOWN";
	}
}

static int st_page_write(uint16_t idx, uint16_t offset, 
	uint8_t *buf, uint16_t len)
{
	st_ctx_t *ctx = &st_ctx;
	uint32_t addr = idx * ctx->header.page_size + ST_PAGE_HEADER_SIZE + offset;
	uint16_t bytes_remain = len;
	uint16_t bytes_write = 0;
	if (!ST_PAGE_VALID(idx)){
		return -1;
	}
	if (offset + len  + ST_PAGE_HEADER_SIZE >= ctx->header.page_size){
		return -1;
	}
	while(bytes_remain){
		bytes_write = MIN(EEPROM_PAGE_SIZE, bytes_remain);
		if (EEPROM_WRITE(addr, buf, bytes_write)){
			LOG_ERROR(TAG, "EEPROM write err.");
			return -1;
		}
		addr += bytes_write;
		bytes_remain -= bytes_write;
	}
	return 0;
}

static int st_page_read(uint16_t idx, uint16_t offset, 
	uint8_t *buf, uint16_t len)
{
	st_ctx_t *ctx = &st_ctx;
	uint32_t addr = idx * ctx->header.page_size + ST_PAGE_HEADER_SIZE + offset;
	if (!ST_PAGE_VALID(idx)){
		LOG_ERROR(TAG, "invalid idx.");
		return -1;
	}
	if (offset + len + ST_PAGE_HEADER_SIZE >= ctx->header.page_size){
		LOG_ERROR(TAG, "invalid offset or len.");
		return -1;
	}
	if (EEPROM_READ(addr, buf, len)){
		LOG_ERROR(TAG, "EEPROM write err.");
		return -1;
	}
	return 0;
}
static int st_page_get_freesize(st_ctx_t *ctx, uint16_t idx){
	if (!ST_PAGE_VALID(idx)){
		return -1;
	}
	switch(ctx->pages[idx].status){
		case ST_PAGE_S_ERASED:
		case ST_PAGE_S_FULL:
		case ST_PAGE_S_UNAVAILABLE:
			break;
		case ST_PAGE_S_ACTIVE:
			return ST_PAGE_CONTENT_MAX - ctx->pages[idx].bytes_used;
		default:
			return -1;
	}
	return 0;
}
static int st_page_erase(st_ctx_t *ctx, uint16_t idx){
	if (!ST_PAGE_VALID(idx)){
		return -1;
	}
	if (ST_PAGE_S_ERASED == ctx->pages[idx].status){
		return 0;
	}
	LOG_DEBUG(TAG, "erase page %d", idx);
	if (EEPROM_ERASE(idx * ctx->header.page_size, ctx->header.page_size)){
		LOG_ERROR(TAG, "EEPROM erase err.");
		return -1;
	}
	ctx->pages[idx].bytes_used = 0;
	ctx->pages[idx].item_cnt = 0;
	ctx->pages[idx].status = ST_PAGE_S_ERASED;
	ctx->page_erased ++;
	return 0;
}
//static int st_page_clear(uint16_t idx, uint16_t offset, 
//	uint16_t len)
//{
//	st_ctx_t *ctx = &st_ctx;
//	uint8_t buf[EEPROM_PAGE_SIZE] = {0};
//	uint32_t addr = idx * ctx->header.page_size + ST_PAGE_HEADER_SIZE + offset;
//	uint16_t bytes_remain = len;
//	uint16_t bytes_write = 0;
//	if (!ST_PAGE_VALID(idx)){
//		return -1;
//	}
//	if (offset + len + ST_PAGE_HEADER_SIZE >= ctx->header.page_size){
//		return -1;
//	}
//	while(bytes_remain){
//		bytes_write = MIN(bytes_remain, EEPROM_PAGE_SIZE);
//		if (EEPROM_WRITE(addr, buf, bytes_write)){
//			LOG_ERROR(TAG, "EEPROM write err.");
//			return -1;
//		}
//		addr += bytes_write;
//		bytes_remain -= bytes_write;
//	}
//	return 0;
//}
static int st_page_status_update(st_ctx_t *ctx, uint16_t idx, uint8_t status){
	st_page_t *page = NULL;
	if (!ST_PAGE_VALID(idx)){
		return -1;
	}
	page = &ctx->pages[idx];
	if (status == page->status){
		return 0;
	}
	LOG_DEBUG(TAG, "page %d status change(%s -> %s)", idx, 
		st_page_status_str(page->status), 
		st_page_status_str(status));
	if (EEPROM_WRITE(idx * ctx->header.page_size, &status, 1)){
		LOG_ERROR(TAG, "EEPROM write err.");
		return -1;
	}
	if (ST_PAGE_S_ERASED == page->status){
		if (ctx->page_erased){
			ctx->page_erased --;
		}
	}
	if (ST_PAGE_S_ACTIVE == page->status){
		ctx->page_active = ST_PAGE_MAX;
	}
	if (ST_PAGE_S_ACTIVE == status){
		ctx->page_active = idx;
	}
	page->status = status;
	return 0;
}
static int st_page_erase_check(uint16_t idx){
	st_ctx_t *ctx = &st_ctx;
	int i = 0;
	uint8_t buf[EEPROM_PAGE_SIZE];
	memset(buf, 0xff, EEPROM_PAGE_SIZE);
	uint32_t bytes_remain = ctx->header.page_size;
	uint32_t bytes_verify = 0;
	uint32_t addr = idx * ctx->header.page_size;
	if (!ST_PAGE_VALID(idx)){
		return -1;
	}
	while(bytes_remain){
		bytes_verify = MIN(EEPROM_PAGE_SIZE, bytes_remain);
		if (EEPROM_READ(addr, buf, bytes_verify)){
			LOG_ERROR(TAG, "read err.");
			goto fail;
		}
		for (i = 0; i < bytes_verify; i++){
			if (buf[i] != 0xff){
				return 0;
			}
		}
		bytes_remain -= bytes_verify;
	}
	return 1;
	
fail:
	return -1;
}
static int st_page_status(st_ctx_t *ctx, uint16_t idx){
	st_page_t *page = NULL;
	if (!ST_PAGE_VALID(idx)){
		return -1;
	}
	page = &ctx->pages[idx];
	return page->status;
}

static int st_page_next(uint16_t idx){
	if (!ST_PAGE_VALID(idx)){
		return -1;
	}
	if (ST_PAGE_VALID(idx + 1)){
		return idx + 1;
	}
	return 0;
}
static int st_page_last(uint16_t idx){
	if (!ST_PAGE_VALID(idx)){
		return -1;
	}
	if (ST_PAGE_VALID(idx - 1)){
		return idx - 1;
	}
	return ST_PAGE_MAX - 1;
}
static int st_first_page(st_ctx_t *ctx, uint16_t page_from, uint8_t status){
	uint16_t i = 0;
	int page_start = page_from;
	if (!ST_PAGE_VALID(page_start)){
		return -1;
	}
	i = page_start;
	do{
		if (ctx->pages[i].status == status){
			return i;
		}
		i = st_page_next(i);
	}while(i != page_start);
	return ST_PAGE_MAX;
}

static int st_item_verify(st_item_t *item){
	crc_type_t crc_type = CRC8_MAXIM_INIT;
	uint32_t crc_value = 0;
	if (!item){
		return 0;
	}
	crc_value = crc_check(&crc_type, (const uint8_t *)item->content, 
		item->header.len);
	return (crc_value == item->header.crc);
}

static int st_page_write_item(st_ctx_t *ctx, uint16_t idx, st_item_t *item){
	st_page_t *page = NULL;
	uint8_t page_status = 0;
//	uint16_t offset = 0;
	int bytes_write = 0;
	int ret = 0;
	if (!ST_PAGE_VALID(idx)){
		return -1;
	}
	if (!ST_ITEM_IDX_VALID(item->header.idx)){
		LOG_ERROR(TAG, "invalid item idx.");
		return -1;
	}
	page = &ctx->pages[idx];
	switch(page->status){
		case ST_PAGE_S_FULL:
		case ST_PAGE_S_UNAVAILABLE:
			goto fail;
		case ST_PAGE_S_ERASED:
		case ST_PAGE_S_ACTIVE:
			break;
		default:
			LOG_ERROR(TAG, "invalid page status.");
			goto fail;
	}
	LOG_DEBUG(TAG, "write item(idx:%04X,len:%d,crc:%02X) to page %d, offset:%d", 
		item->header.idx, item->header.len, item->header.crc, idx, 
		page->bytes_used);
	bytes_write = ST_ITEM_SIZE(item);
	ret = st_page_write(idx, page->bytes_used, (uint8_t *)item, bytes_write);
	if (ret < 0){
		LOG_ERROR(TAG, "fail to write item.");
		goto fail;
	}
	page->bytes_used += bytes_write;
	page->bytes_available += bytes_write;
	page_status = page->status;
	if (ST_PAGE_S_ERASED == page->status){
		page_status = ST_PAGE_S_ACTIVE;
	}
	if (ST_PAGE_CONTENT_MAX - page->bytes_used < sizeof(st_item_header_t)){
		page_status = ST_PAGE_S_FULL;
	}
	if (page_status != page->status){
		ret = st_page_status_update(ctx, idx, page_status);
		if (ret < 0){
			LOG_ERROR(TAG, "fail to update page status.");
			goto fail;
		}
	}
	page->item_cnt ++;
	return 0;
fail:
	return -1;
}

static int st_page_realign(st_ctx_t *ctx, uint16_t idx){
	st_page_t *page = NULL;
	st_item_t item = {0};
	uint16_t offset = 0;
	int ret = 0;
	page = &ctx->pages[idx];
//	uint32_t addr = idx * ctx->header.page_size;
//	uint32_t bytes_read = sizeof(item.header);
	LOG_DEBUG(TAG, "page %d realign", idx);
	while(offset < page->bytes_used){
		ret = st_page_read(idx, offset, (uint8_t *)&item.header, 
			sizeof(st_item_header_t));
		if (ret < 0){
			LOG_ERROR(TAG, "fail to read item header.");
			goto fail;
		}
		if (!item.header.crc || !item.header.idx){
			offset += item.header.len + sizeof(st_item_header_t);
			continue;
		}
		ret = st_page_read(idx, offset + sizeof(st_item_header_t), 
			item.content, item.header.len);
		if (ret < 0){
			LOG_ERROR(TAG, "fail to read item content.");
			goto fail;
		}
		if (!st_item_verify(&item)){
			LOG_ERROR(TAG, "item verify failed.");
			item.header.idx = 0;
			st_page_write(idx, offset, (uint8_t *)&item.header, sizeof(st_item_header_t));
			offset += item.header.len + sizeof(st_item_header_t);
			if (page->item_cnt){
				page->item_cnt --;
			}
			continue;
		}
		if (st_page_get_freesize(ctx, ctx->page_active) < ST_ITEM_SIZE(&item)){
			uint16_t page_new = st_first_page(ctx, ctx->page_active, ST_PAGE_S_ERASED);
			if (!ST_PAGE_VALID(page_new)){
				LOG_ERROR(TAG, "no enougn page.");
				goto fail;
			}
			st_page_status_update(ctx, ctx->page_active, ST_PAGE_S_FULL);
			st_page_status_update(ctx, page_new, ST_PAGE_S_ACTIVE);
		}
		ret = st_page_write_item(ctx, ctx->page_active, &item);
		if (ret < 0){
			LOG_ERROR(TAG, "fail to write item.");
			goto fail;
		}
		offset += ST_ITEM_SIZE(&item);
	}
	ret = st_page_erase(ctx, idx);
	if (ret < 0){
		LOG_ERROR(TAG, "erase err.");
		goto fail;
	}
	return 0;
fail:
	return -1;
}


static int st_realign(st_ctx_t *ctx){
	int i = 0;
	int ret = 0;
	uint16_t page_start = ST_PAGE_MAX;
	uint16_t page_idx = ST_PAGE_MAX;
	if (!ctx){
		LOG_ERROR(TAG, "%s:param err", __FUNCTION__);
		return -1;
	}
	page_start = ctx->page_active;
	if (!ST_PAGE_VALID(page_start)){
		page_start = 0;
	}
	i = page_start;
	do {
		if (ST_PAGE_S_FULL == ctx->pages[i].status){
			if (!ST_PAGE_VALID(page_idx)){
				page_idx = i;
			}else{
				if (ctx->pages[i].item_cnt < ctx->pages[page_idx].item_cnt){
					page_idx = i;
				}
			}
		}
		i = st_page_next(i);
	}while(i != page_start);
	if (!ST_PAGE_VALID(page_idx)){
		LOG_DEBUG(TAG, "%s:page not found", __FUNCTION__);
		return 0;
	}
	ret = st_page_realign(ctx, page_idx);
	if (ret < 0){
		LOG_ERROR(TAG, "page %d realign failed", page_idx);
	}
	return 0;
}

static int st_page_delete_item(st_ctx_t *ctx, uint16_t idx, uint16_t item_idx)
{
	st_page_t *page = NULL;
	st_item_t item = {0};
	uint16_t offset = 0;
	int cnt = 0;
	int ret = 0;
	if (!ST_PAGE_VALID(idx)){
		return -1;
	}
	if (!ST_ITEM_IDX_VALID(item_idx)){
		LOG_ERROR(TAG, "invalid item idx.");
		return -1;
	}
	page = &ctx->pages[idx];
	if (ST_PAGE_S_ERASED == page->status || 
		ST_PAGE_S_UNAVAILABLE == page->status )
	{
		return 0;
	}
	while(offset < page->bytes_used){
		ret = st_page_read(idx, offset, (uint8_t *)&item.header, 
			sizeof(st_item_header_t));
		if (ret < 0){
			LOG_ERROR(TAG, "fail to read item header.");
			goto fail;
		}
		if (!item.header.idx){
			offset += item.header.len + sizeof(st_item_header_t);
			continue;
		}
		if (item.header.idx != item_idx){
			offset += item.header.len + sizeof(st_item_header_t);
			continue;
		}
		LOG_DEBUG(TAG, "delete item(idx:%04X) from page %d, offset:%d", 
			item.header.idx, idx, offset);
		item.header.idx = 0;
		ret = st_page_write(idx, offset, (uint8_t *)&item.header, 
			sizeof(st_item_header_t));
		if (ret < 0){
			LOG_ERROR(TAG, "fail to write item header.");
			goto fail;
		}
		if (page->item_cnt){
			page->item_cnt -= 1;
			page->bytes_available -= ST_ITEM_SIZE(&item);
		}
		cnt ++;
		offset += ST_ITEM_SIZE(&item);
	}
	if (!page->item_cnt && ST_PAGE_S_FULL == page->status){
		ret = st_page_erase(ctx, idx);
		if (ret < 0){
			LOG_ERROR(TAG, "erase failed.");
		}
	}
	return cnt;
fail:
	return -1;
}
int st_delete_item_with_loc(st_ctx_t *ctx, uint16_t item_idx, st_item_location_t *location){
	st_page_t *page = NULL;
	st_item_header_t header = {0};
	int ret = 0;
	if (!location){
		LOG_ERROR(TAG, "%s:param err", __FUNCTION__);
		goto fail;
	}
	if (!ST_PAGE_VALID(location->page)){
		LOG_ERROR(TAG, "%s:invalid page idx(%d)", __FUNCTION__, location->page);
		goto fail;
	}
	page = &ctx->pages[location->page];
	if (!page->item_cnt){
		LOG_DEBUG(TAG, "%s:target page is empty", __FUNCTION__);
		goto out;
	}
	ret = st_page_read(location->page, location->offset, (uint8_t *)&header, 
		sizeof(st_item_header_t));
	if (ret < 0){
		LOG_ERROR(TAG, "%s:fail to read header", __FUNCTION__);
		goto fail;
	}
	if (header.idx != item_idx){
		LOG_ERROR(TAG, "%s:idx not match, expect %04X but %04X found", __FUNCTION__,
			item_idx, header.idx);
		goto fail;
	}
	LOG_DEBUG(TAG, "delete item(idx:%04X) from page %d, offset:%d", 
			item_idx, location->page, location->offset);
	header.idx = 0;
	ret = st_page_write(location->page, location->offset, (uint8_t *)&header, 
		sizeof(st_item_header_t));
	if (ret < 0){
		LOG_ERROR(TAG, "%s:fail to write header", __FUNCTION__);
		goto fail;
	}
	if (page->item_cnt){
		page->item_cnt --;
	}
	if (!page->item_cnt && ST_PAGE_S_FULL == page->status){
		ret = st_page_erase(ctx, location->page);
		if (ret < 0){
			LOG_ERROR(TAG, "erase err.");
			goto fail;
		}
	}
out:
	return 0;
fail:
	return -1;
}
static int st_page_find_item(st_ctx_t *ctx, uint16_t idx, 
	st_item_t *result)
{
	st_page_t *page = NULL;
	st_item_t item = {0};
	uint16_t offset = 0;
	int cnt = 0;
	int ret = 0;
	if (!ctx){
		return -1;
	}
	if (!ST_PAGE_VALID(idx)){
		return -1;
	}
	if (!ST_ITEM_IDX_VALID(ctx->search_ctx.item_idx)){
		LOG_ERROR(TAG, "invalid item idx.");
		return -1;
	}
	page = &ctx->pages[idx];
	if (ST_PAGE_S_ERASED == page->status || 
		ST_PAGE_S_UNAVAILABLE == page->status)
	{
		return 0;
	}
	while(offset < page->bytes_used){
		ret = st_page_read(idx, offset, (uint8_t *)&item.header, 
			sizeof(st_item_header_t));
		if (ret < 0){
			LOG_ERROR(TAG, "fail to read item header.");
			goto fail;
		}
		if (!item.header.idx){
			offset += item.header.len + sizeof(st_item_header_t);
			continue;
		}
		if (item.header.idx != ctx->search_ctx.item_idx){
			offset += item.header.len + sizeof(st_item_header_t);
			continue;
		}
		LOG_DEBUG(TAG, "find idx %04X in page %d with offset %d", item.header.idx, 
			idx, offset);
		ret = st_page_read(idx, offset + sizeof(st_item_header_t), 
			item.content, item.header.len);
		if (ret < 0){
			LOG_ERROR(TAG, "fail to read item content.");
			goto fail;
		}
		if (!st_item_verify(&item)){
			LOG_ERROR(TAG, "item verify failed.");
			item.header.idx = 0;
			st_page_write(idx, offset, (uint8_t *)&item.header, sizeof(st_item_header_t));
			offset += item.header.len + sizeof(st_item_header_t);
			continue;
		}
		if (result){
			memcpy(result, &item, sizeof(st_item_t));
		}
		//delete last record if exist;
		if (ST_PAGE_VALID(ctx->search_ctx.last_location.page)){
			ret = st_delete_item_with_loc(ctx, ctx->search_ctx.item_idx, 
				&ctx->search_ctx.last_location);
			if (ret < 0){
				LOG_ERROR(TAG, "fail to delete last record");
				goto fail;
			}
		}
		ctx->search_ctx.last_location.page = idx;
		ctx->search_ctx.last_location.offset = offset;
		memcpy(&ctx->search_ctx.last_header, (uint8_t *)&item.header, 
			sizeof(st_item_header_t));
		cnt ++;
		offset += item.header.len + sizeof(st_item_header_t);
	}
	return cnt;
fail:
	return -1;
}

static worktime_t lasttime;

/**
 * @brief scan whole page
 * @param ctx Storage context
 * @param idx page index
 * @return 0-success, -1 - error
 */
static int st_page_scan(st_ctx_t *ctx, uint16_t idx){
	st_page_t *page = NULL;
	st_item_t item = {0};
	int ret = 0;
	page = &ctx->pages[idx];
//	uint32_t addr = idx * ctx->header.page_size;
//	uint32_t bytes_read = sizeof(item.header);
	page->bytes_used = 0;
	page->item_cnt = 0;
	LOG_DEBUG(TAG, "scan page %d", idx);
	while(page->bytes_used < ST_PAGE_CONTENT_MAX){
		//read item content
		ret = st_page_read(idx, page->bytes_used, 
			(uint8_t *)&item.header, sizeof(item.header));
		if (ret < 0){
			LOG_ERROR(TAG, "fail to read item header");
			goto fail;
		}
		if (0xFFU == item.header.len){
			break;
		}
		if (ST_ITEM_IDX_VALID(item.header.idx)){
			LOG_DEBUG(TAG, "item len:%d, idx:0x%04x, crc:%02x", item.header.len,
				item.header.idx, item.header.crc);
			//load item content
			ret = st_page_read(idx, page->bytes_used + sizeof(st_item_header_t), 
				item.content, item.header.len);
			if (ret < 0){
				LOG_ERROR(TAG, "fail to read item content.");
				goto fail;
			}
			// verify item content, mark it erased if content is corrupt.
			if (!st_item_verify(&item)){
				LOG_ERROR(TAG, "item verify failed.");
				item.header.idx = 0;
				st_page_write(idx, page->bytes_used, (uint8_t *)&item.header, sizeof(st_item_header_t));
				page->bytes_used += item.header.len + sizeof(st_item_header_t);
				continue;
			}
			page->bytes_available += item.header.len + sizeof(st_item_header_t);
			page->item_cnt ++;
		}
		page->bytes_used += item.header.len + sizeof(st_item_header_t);
	}
	if (page->bytes_used >= ST_PAGE_CONTENT_MAX){
		st_page_status_update(ctx, idx, ST_PAGE_S_FULL);
	}
	// erase page if status is FULL and all item was marked as erased
	if (ST_PAGE_S_FULL == page->status && !page->item_cnt){
		ret = st_page_erase(ctx, idx);
		if (ret < 0){
			LOG_ERROR(TAG, "erase err.");
			goto fail;
		}
	}
	LOG_DEBUG(TAG, "page %d scan finish, %d bytes used, %d bytes available", 
		idx, page->bytes_used, page->bytes_available);
	return 0;
fail:
	return -1;
}

static int st_page_init(st_ctx_t *ctx){
	st_page_t *page = NULL;
	int ret = 0;
	uint16_t idx = ST_PAGE_MAX;

	for(idx = 0; idx < ST_PAGE_MAX; idx++){
		page = &ctx->pages[idx];
		memset(page, 0, sizeof(st_page_t));
		if (EEPROM_READ(idx * ctx->header.page_size, &page->status, sizeof(page->status))){
			LOG_ERROR(TAG, "fail to read page status");
			goto fail;
		}
		//LOG_DEBUG(TAG, "page %d: %s", idx, st_page_status_str(page->status));
		switch(page->status){
			case ST_PAGE_S_UNAVAILABLE:
				break;
			case ST_PAGE_S_ERASED:
				ret = st_page_erase_check(idx);
				if (ret < 0){
					LOG_ERROR(TAG, "erase check err.");
					goto fail;
				}
				if (!ret){
					LOG_ERROR(TAG, "erace check failed. erase again");
					if (st_page_erase(ctx, idx) < 0){
						LOG_ERROR(TAG, "erase err");
						goto fail;
					}
				}
				ctx->page_erased ++;
				break;
			case ST_PAGE_S_FULL:
			case ST_PAGE_S_ACTIVE:
				if (st_page_scan(ctx, idx)){
					LOG_ERROR(TAG, "fail to load page");
					goto fail;
				}
				ctx->page_active = idx;
				break;
			default:
				LOG_ERROR(TAG, "unknown status(%02x), erase whole page", page->status);
				if (st_page_erase(ctx, idx) < 0){
					LOG_ERROR(TAG, "erase err");
					goto fail;
				};
				page->status = 0xff;
		}
	}
	LOG_DEBUG(TAG, "page init finish, erased:%d, active:%d", ctx->page_erased,
		ctx->page_active);
	return 0;
fail:
	return -1;
}

int st_erase_all(st_ctx_t *ctx){
	int i = 0;
	int ret = 0;
//	st_page_status_t page_status = {0};
	for(i = 0; i < ST_PAGE_MAX; i++){
		if (ST_PAGE_S_ERASED == ctx->pages[i].status){
			continue;
		}
		ret = st_page_erase(ctx, i);
		if (ret < 0){
			LOG_ERROR(TAG, "%s:fail to erase page %d", __FUNCTION__, i);
			goto fail;
		}
	}
	ctx->page_active = ST_PAGE_MAX;
	return 0;
fail:
	return -1;
}

static int st_header_init(st_ctx_t *ctx){
	crc_type_t crc_type = CRC16_MAXIM_INIT;
	memset(&ctx->header, 0, sizeof(st_header_t));
	ctx->header.version = ST_VERSION;
	ctx->header.page_size = ST_PAGE_SIZE;
	ctx->header.crc = crc_check(&crc_type, (const uint8_t *)(&ctx->header) + 2, 
		sizeof(st_header_t) - 2);
	if (EEPROM_WRITE(ST_HEADER_OFFSET, &ctx->header, sizeof(st_header_t))){
		LOG_ERROR(TAG, "fail to wirte header");
		goto fail;
	}
	return 0;
fail:
	return -1;
}

static int st_erase(){
	if (EEPROM_ERASE(0, EEPROM_MAX_SIZE)){
		LOG_ERROR(TAG, "erase err");
		goto fail;
	}
	return 0;
fail:
	return -1;
}

static int st_header_verify(st_header_t *header){
	crc_type_t crc_type = CRC16_MAXIM_INIT;
	uint32_t crc_value = 0;
	if (!header){
		return 0;
	}
	crc_value = crc_check(&crc_type, (const uint8_t *)&header->version, 
		sizeof(st_header_t) - sizeof(header->crc));
	
	return (crc_value == header->crc);
}

static int st_load_header(st_ctx_t *ctx){
	if (EEPROM_READ(ST_HEADER_OFFSET, &ctx->header, ST_RESERVE_SIZE)){
		LOG_ERROR(TAG, "fail to read storage header");
		goto fail;
	}
	if (!st_header_verify(&ctx->header)){
		LOG_ERROR(TAG, "header verify failed");
		goto fail;
	}
	return 0;
fail:
	memset(&ctx->header, 0, sizeof(st_header_t));
	return -1;
}

/**
 * @brief Storage module init. Must called before read or write data.
 * @return 0-Success, (-1)-Error
 */

int st_init(){
	int ret = 0;
//	st_page_status_t page_status = {0};
	st_ctx.page_start = ST_PAGE_MAX;
	st_ctx.page_active = ST_PAGE_MAX;
	lasttime = worktime_get();
	ret = st_load_header(&st_ctx);
	if (ret < 0){
		LOG_INFO(TAG, "erase all storage");
		st_erase();
		ret = st_header_init(&st_ctx);
		if (ret < 0){
			goto fail;
		}
	}
	LOG_DEBUG(TAG, "header init, version:%04X, page_size:%d", 
		st_ctx.header.version, st_ctx.header.page_size);
	ret = st_page_init(&st_ctx);
	if (ret < 0){
		LOG_INFO(TAG, "erase all page");
		st_erase_all(&st_ctx);
	}
	st_ctx.flag_init = 1;
	
//out:
	return 0;
fail:
	return -1;
}

static int st_find_item(uint16_t item_idx, st_item_t *result, 
	st_item_location_t *location)
{
	st_ctx_t *ctx = &st_ctx;
	uint16_t page_start = ctx->page_active;
	int i = 0;
	int ret, cnt;
	ret = cnt = 0;
	if (!ST_ITEM_IDX_VALID(item_idx)){
		goto fail;
	}
	if (!ST_PAGE_VALID(page_start)){
		page_start = 0;
	}
	ctx->search_ctx.item_idx = item_idx;
	ctx->search_ctx.last_location.page = ST_PAGE_MAX;
	i = page_start; 
	do{
		if (ST_PAGE_S_FULL == st_page_status(ctx, i) || 
			ST_PAGE_S_ACTIVE == st_page_status(ctx, i))
		{
			if (cnt){
				result = NULL;
			}
			ret = st_page_find_item(ctx, i, result);
			if (ret < 0){
				goto fail;
			}
			cnt += ret;
			if (1 == cnt && location){
				memcpy(location, &ctx->search_ctx.last_location, 
					sizeof(st_item_location_t));
			}
			if (cnt > 1){
				ret = st_delete_item_with_loc(ctx, item_idx, &ctx->search_ctx.last_location);
				if (ret < 0){
					LOG_ERROR(TAG, "fail to delete item");
					goto fail;
				}
			}
		}
		i = st_page_last(i);
	}while(i != page_start);
	
	return cnt;
fail:
	return -1;
}

static int st_get_available_page(st_ctx_t *ctx, uint16_t data_len){
//	int i = 0;
	int ret = 0;
	uint16_t page_idx = ST_PAGE_MAX;
//	st_page_status_t page_status = {0};
	if (!ctx){
		return -1;
	}
	if (data_len + sizeof(st_item_header_t) > ST_MAX_CONTENT_LEN){
		return -1;
	}
	if (ST_PAGE_VALID(ctx->page_active)){
		page_idx = ctx->page_active;
	}else{
		page_idx = st_first_page(ctx, 0, ST_PAGE_S_ACTIVE);
	}
	if (ST_PAGE_VALID(page_idx)){
		if (st_page_get_freesize(ctx, page_idx) >= 
			data_len + sizeof(st_item_header_t))
		{
			return page_idx;
		}
		if (ctx->page_erased <= 2){
			st_realign(ctx);
			//realign may change active page
			page_idx = ctx->page_active;
			if (st_page_get_freesize(ctx, page_idx) >= 
				data_len + sizeof(st_item_header_t))
			{
				return page_idx;
			}
		}
		ret = st_page_status_update(ctx, page_idx, ST_PAGE_S_FULL);
		if (ret < 0){
			LOG_ERROR(TAG, "fail to upadate status");
			goto fail;
		}
	}else{
		page_idx = 0;
	}
	
	page_idx = st_first_page(ctx, page_idx, ST_PAGE_S_ERASED);
	if (!ST_PAGE_VALID(page_idx)){
		LOG_ERROR(TAG, "no enough space");
		goto fail;
	}
	ret = st_page_status_update(ctx, page_idx, ST_PAGE_S_ACTIVE);
	if (ret < 0){
		LOG_ERROR(TAG, "fail to upadate status");
		goto fail;
	}
	return page_idx;
fail:
	return -1;
}

/**
 * @brief write item to data storage
 * @param item_idx index of data item
 * @param buf pointer to hold item data
 * @param len length of "buf"
 * @return (-1)-ERROR, 0-success
 */

int st_write_item(uint16_t item_idx, uint8_t *buf, int len)
{
	st_item_t item = {0};
	st_item_location_t location = {0};
	crc_type_t crc_type = CRC8_MAXIM_INIT;
	uint16_t page_idx = ST_PAGE_MAX;
	st_page_t *page = NULL;
	st_ctx_t *ctx = &st_ctx;
//	int i = 0;
	int ret = 0;
	if (!buf || len <= 0 || len > ST_MAX_CONTENT_LEN){
		return -1;
	}
	if (!ST_ITEM_IDX_VALID(item_idx)){
		return -1;
	}
	location.page = ST_PAGE_MAX;
	ret = st_find_item(item_idx, &item, &location);
	if (ret < 0){
		LOG_ERROR(TAG, "fail to find item");
		goto fail;
	}
	
	item.header.idx = item_idx;
	item.header.len = len;
	memcpy(item.content, buf, len);
	item.header.crc = crc_check(&crc_type, (const uint8_t *)item.content, len);
	page_idx = st_get_available_page(ctx, len);
	if (!ST_PAGE_VALID(page_idx)){
		LOG_ERROR(TAG, "no available page");
		goto fail;
	}
	page = &ctx->pages[page_idx];
	ret = st_page_write_item(ctx, page_idx, &item);
	if (ret < 0){
		LOG_ERROR(TAG, "fail to write new item");
		goto fail;
	}
	if (ST_PAGE_S_ACTIVE == page->status){
		ctx->page_active = page_idx;
	}
	if (ST_PAGE_VALID(location.page)){
		ret = st_delete_item_with_loc(ctx, item_idx, &location);
		if (ret < 0){
			LOG_ERROR(TAG, "fail to delete old item");
			goto fail;
		}
	}
	return 0;
fail:
	return -1;
}

/**
 * @brief read item from data storage
 * @param item_idx index of data item
 * @param buf pointer to hold item data
 * @param len length of "buf"
 * @return (-1)-ERROR, 0-item not found, (>0)-length(in bytes) of data read
 */
int st_read_item(uint16_t item_idx, uint8_t *buf, int len)
{
	st_item_t item = {0};
//	st_page_t *page = NULL;
//	int i = 0;
	int ret = 0;
	if (!st_is_init()){
		return -1;
	}
	if (!buf || len <= 0){
		return -1;
	}
	if (!ST_ITEM_IDX_VALID(item_idx)){
		return -1;
	}
	ret = st_find_item(item_idx, &item, NULL);
	if (ret < 0){
		goto fail;
	}else if (0 == ret){
		return 0;
	}
	memcpy(buf, item.content, MIN(len, item.header.len));
	return MIN(len, item.header.len);
fail:
	return -1;
}

int st_delete_item(uint16_t item_idx){
	st_ctx_t *ctx = &st_ctx;
	int i = 0;
	int ret, cnt;
	ret = cnt = 0;
	if (!ST_ITEM_IDX_VALID(item_idx)){
		goto fail;
	}
	for(i = ctx->page_start; st_page_next(i) != ctx->page_start; 
		i = st_page_next(i))
	{
		if (ST_PAGE_S_ERASED == st_page_status(ctx, i)){
			break;
		}
		ret = st_page_delete_item(ctx, i, item_idx);
		if (ret < 0){
			goto fail;
		}
		cnt += ret;
	}
	return cnt;
fail:
	return -1;
}

