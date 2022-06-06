using System;
using System.ComponentModel.Composition;

namespace Calculator.Extension.AIS3333
{
    [Export(typeof(IOperation))]
    [ExportMetadata("Symbol", 'k')]
    public class AIS3333 : IOperation
    {
        private ICalculator _calculator;

        [ImportingConstructor]
        public AIS3333(ICalculator calculator)
        {
            _calculator = calculator;
        }

        public int Operate(string left, string right)
        {
            try
            {
                int[] key = { 9, 52, 8, 13, 7, 5, 48, 87, 0 };
                int value;

                if (right[6] != '_')
                {
                    throw new Exception();
                }
                
                if (right[5] != right[8])
                {
                    throw new Exception();
                }

                if (right.Length != 18)
                {
                    throw new Exception();
                }
                
                if (right[3] != right[6])
                {
                    throw new Exception();
                }
                
                value = int.Parse(_calculator.Calculate("1+" + right.Substring(1, 2)));

                if (right[7] != 'C')
                {
                    throw new Exception();
                }

                if (right[8] != '0')
                {
                    throw new Exception();
                }

                if (value != 16)
                {
                    throw new Exception();
                }
                
                if (right[6] != right[0])
                {
                    throw new Exception();
                }

                if (right[4] != 'S')
                {
                    throw new Exception();
                }

                for (int i = 0; i < key.Length; ++i)
                {
                    if ((right[9 + i] ^ 100) != key[i])
                    {
                        throw new Exception();
                    }
                }

                return 7;
            }
            catch (Exception e)
            {
                throw e;
            }
        }
    }
}
