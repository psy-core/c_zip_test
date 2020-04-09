#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

// gzCompress: do the compressing
int gzCompress(const char *src, int srcLen, char *dest, int destLen)
{
    z_stream c_stream;
    int err = 0;

    //不同包装格式的数据输出
    // 8 ~ 15: 输出 zlib 数据头/尾, deflateInit() 中这个参数值固定为 15, 就是 zconf.h 中定义的 MAX_WBITS 的值.
    // -8 ~ -15: 输出原始的压缩数据不含任何数据头/尾. 如果没有特殊要求,使用 -15 就可以,表示内部使用 32K 的 LZ77 滑动窗口.
    // 24 ~ 31: 输出 gzip 格式的数据,默认提供一个所有设置都清零的 gzip 数据头,如果要自定义这个数据头,可以在初始化之后, deflate() 之前调用 deflateSetHeader().
    int windowBits = 15;
    int GZIP_ENCODING = 16;

    if (src && srcLen > 0)
    {
        // c_stream.zalloc = (alloc_func)0;
        // c_stream.zfree = (free_func)0;
        // c_stream.opaque = (voidpf)0;
        c_stream.zalloc = Z_NULL;
        c_stream.zfree = Z_NULL;
        c_stream.opaque = Z_NULL;

        int ret = deflateInit2(
            &c_stream,
            Z_DEFAULT_COMPRESSION, //-1, 使用默认设置  压缩级别 0 ~ 9. 0 表示不压缩, 1 表示速度最快, 9 表示压缩比最高
            Z_DEFLATED,            //8， 只是唯一支持的压缩算法
            16 + 15,               // 16 makes it a gzip file, 15 is default
            8,                     //memLevel: 控制 libzlib 内部使用内存的大小, 1 ~ 9 数字越小内存用量也越小,花费时间越多.默认值是8.
            Z_DEFAULT_STRATEGY     //内部压缩算法的编码策略,如果没有特殊要求,设置为 Z_DEFAULT_STRATEGY 就可以了
                                   //如果你有特殊要求,那你自然知道其余选项 Z_FILTERED / Z_HUFFMAN_ONLY / Z_RLE / Z_FIXED 是什么意思
        );

        if (ret != Z_OK)
            return -1;
        
        return 1;

        // c_stream.next_in = (Bytef *)src;
        // c_stream.avail_in = srcLen;
        // c_stream.next_out = (Bytef *)dest;
        // c_stream.avail_out = destLen;

        // //如果没有特殊需求,我们可以先以 flush = Z_NO_FLUSH 调用 deflate()
        // //在输入数据压缩完成之后,还需要以 flush = Z_FINISH 调用并确认 deflate() 返回 Z_STREAM_END
        // //表示所有数据都已经写入到输出缓冲,一个流结束.
        // //如果一次性输入所有原文,那么也可以直接以 flush = Z_FINISH 调用 deflate(),这正是 compress() 的做法.
        // while (c_stream.avail_in != 0 && c_stream.total_out < destLen)
        // {
        //     if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK)
        //         return -1;
        // }
        // if (c_stream.avail_in != 0)
        // return c_stream.avail_in; //不懂,觉得应该是return -1才对
        // for (;;)
        // {
        //     if ((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END)
        //         break;
        //     if (err != Z_OK)
        //         return -1;
        // }
        // if (deflateEnd(&c_stream) != Z_OK)
        //     return -1;
        // return c_stream.total_out;
    }
    return -1;
}

// gzDecompress: do the decompressing
int gzDecompress(const char *src, int srcLen, const char *dst, int dstLen)
{
    z_stream strm;
    strm.zalloc = NULL;
    strm.zfree = NULL;
    strm.opaque = NULL;

    strm.avail_in = srcLen;
    strm.avail_out = dstLen;
    strm.next_in = (Bytef *)src;
    strm.next_out = (Bytef *)dst;

    int err = -1, ret = -1;
    //和 deflateInit2 对应,通常用相同的 windowBits 值就可以了.把 windowBits + 32 可以使解压器自动识别 zlib 或者 gzip 包装格式.
    err = inflateInit2(&strm, MAX_WBITS + 16);
    if (err == Z_OK)
    {
        //解压缩,和 deflate 的调用流程一样,最后应该以参数 flush = Z_FINISH 调用 infate,
        //返回 Z_STREAM_END 表示解压缩完成,并且校验和匹配.
        err = inflate(&strm, Z_FINISH);
        if (err == Z_STREAM_END)
        {
            ret = strm.total_out;
        }
        else
        {
            inflateEnd(&strm);
            return err;
        }
    }
    else
    {
        inflateEnd(&strm);
        return err;
    }
    inflateEnd(&strm);
    return err;
}

// int main()
// {
//     char *src = "just for test, dd, dd, dd, dddddddddddddddddddddddddddddddddd";
//     int size_src = strlen(src);
//     char *compressed = malloc(size_src * 2);
//     memset(compressed, 0, size_src * 2);
//     printf("to compress src: %s\n", src);
//     printf("to compress src size: %d\n", size_src);

//     int gzSize = gzCompress(src, size_src, compressed, size_src * 2);
//     if (gzSize <= 0)
//     {
//         printf("compress error.\n");
//         return -1;
//     }
//     // printf("compressed: ");
//     // int i = 0;
//     // for (; i < gzSize; ++i)
//     // {
//     //     printf("%02x ", compressed[i]);
//     // }
//     printf("\ncompressed size: %d\n", gzSize);

//     char *uncompressed = malloc(size_src * 2);
//     memset(uncompressed, 0, size_src * 2);
//     int ret = gzDecompress(compressed, gzSize, uncompressed, size_src * 2);
//     printf("uncompressed: %s\n", uncompressed);
//     printf("uncompressed size: %lu\n", strlen(uncompressed));

//     free(compressed);
//     free(uncompressed);
//     return 0;
// }
