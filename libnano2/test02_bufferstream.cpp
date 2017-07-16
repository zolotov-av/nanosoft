/****************************************************************************

Тест №02: тест класса BufferStream

****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <string>

#include <nanosoft/bufferstream.h>

int test_count;
int fail_count;

const char *test(bool status)
{
	test_count++;
	if ( ! status ) fail_count++;
	return status ? "ok" : "fail";
}

bool check(uint8_t *buf, int buf_len, uint8_t c)
{
	for(int i = 0; i < buf_len; i++)
	{
		if ( buf[i] != c ) return false;
	}
	return true;
}

void test_stream(BlocksPool *bp)
{
	BufferStream buf(bp);
	
	test_count = 0;
	fail_count = 0;
	
	uint8_t data[BLOCKSPOOL_BLOCK_SIZE * 2];
	
	int ret, size;
	
	// чтение из пустого буфера должно вернуть 0
	ret = buf.read(data, sizeof(data));
	printf("read() from empty = %d [ %s ]\n", ret, test(ret == 0));
	
	// запись в буфер при пустом пуле, должно вернуть 0
	ret = buf.write(data, sizeof(data));
	printf("write() to empty pool = %d [ %s ]\n", ret, test(ret == 0));
	
	bp->reserve(16);
	
	// записать в буфер что-нибудь
	std::string s = "Hello world";
	ret = buf.write(s.c_str(), s.size());
	printf("write(%s) = %d [ %s ]\n", s.c_str(), ret, test(ret == s.size()));
	
	// прочитать из буфера
	ret = buf.read(data, sizeof(data));
	data[ret] = 0;
	printf("read(%d) = %d '%s' [ %s ]\n", (int)sizeof(data), ret, data, test(ret == s.size()));
	
	// записать в буфер четверть блока
	size = BLOCKSPOOL_BLOCK_SIZE / 4;
	memset(data, 1, size);
	ret = buf.write(data, size);
	printf("write(%d) = %d [ %s ]\n", size, ret, test(ret == size));
	
	// записать еще целый блок
	memset(data, 1, BLOCKSPOOL_BLOCK_SIZE);
	ret = buf.write(data, BLOCKSPOOL_BLOCK_SIZE);
	printf("write(%d) = %d [ %s ]\n", BLOCKSPOOL_BLOCK_SIZE, ret, test(ret == BLOCKSPOOL_BLOCK_SIZE));
	
	// проверить размер данных в буфере
	printf("getDataSize() = %d [ %s ]\n", (int)buf.getDataSize(), test(buf.getDataSize() == (BLOCKSPOOL_BLOCK_SIZE+size)));
	
	// проверить число занятых блоков
	printf("busy blocks = %d [ %s ]\n", bp->getBusyCount(), test(bp->getBusyCount() == 2));
	
	// прочитать целый блок
	ret = buf.read(data, BLOCKSPOOL_BLOCK_SIZE);
	printf("read(%d) = %d [ %s ]\n", BLOCKSPOOL_BLOCK_SIZE, ret, test(ret == BLOCKSPOOL_BLOCK_SIZE));
	
	// проверить размер данных в буфере
	printf("getDataSize() = %d [ %s ]\n", (int)buf.getDataSize(), test(buf.getDataSize() == size));
	
	// проверить число занятых блоков
	printf("busy blocks = %d [ %s ]\n", bp->getBusyCount(), test(bp->getBusyCount() == 1));
	
	// записать четветь блока
	memset(data, 2, size);
	ret = buf.write(data, size);
	printf("write(%d) = %d [ %s ]\n", size, ret, test(ret == size));
	
	// прочитать четверь блока и проверить данные
	ret = buf.read(data, size);
	printf("read(%d) = %d [ %s ]\n", size, ret, test(ret == size));
	printf("check data [ %s ]\n", test(check(data, size, 1)));
	
	// записать записать еще блок
	memset(data, 3, BLOCKSPOOL_BLOCK_SIZE);
	ret = buf.write(data, BLOCKSPOOL_BLOCK_SIZE);
	printf("write(%d) = %d [ %s ]\n", size, ret, test(ret == BLOCKSPOOL_BLOCK_SIZE));
	
	// прочитать еще четверь блока и проверить данные
	ret = buf.read(data, size);
	printf("read(%d) = %d [ %s ]\n", size, ret, test(ret == size));
	printf("check data [ %s ]\n", test(check(data, size, 2)));
	
	// проверить размер данных в буфере и число занятых блоков
	printf("getDataSize() = %d [ %s ]\n", (int)buf.getDataSize(), test(buf.getDataSize() == BLOCKSPOOL_BLOCK_SIZE));
	printf("busy blocks = %d [ %s ]\n", bp->getBusyCount(), test(bp->getBusyCount() == 2));
}

int main()
{
	printf("test class BufferStream\n");
	
	BlocksPool bp;
	
	test_stream(&bp);
	
	printf("\n");
	printf("bp.total = %d\n", bp.getTotalCount());
	printf("bp.free = %d\n", bp.getFreeCount());
	// проверить число занятых блоков
	printf("bp.busy = %d [ %s ]\n", bp.getBusyCount(), test(bp.getBusyCount() == 0));
	
	printf("\n");
	printf("test result %d of %d [ %s ]\n", (test_count - fail_count), test_count, (fail_count==0 ? "ok" : "fail"));
	
	return 0;
}
