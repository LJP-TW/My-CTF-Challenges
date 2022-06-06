using System;
using System.ComponentModel.Composition;

namespace Calculator.Extension.AIS333
{
    [Export(typeof(IOperation))]
    [ExportMetadata("Symbol", 'D')]
    public class AIS333 : IOperation
    {
        private ICalculator _calculator;

        [ImportingConstructor]
        public AIS333(ICalculator calculator)
        {
            _calculator = calculator;
        }

        public int Operate(string left, string right)
        {
            try
            {
                int[] key = {10, 110, 101, 116, 9, 110, 101, 124, 104, 123, 87, 9, 109, 10, 72};

                if (right.Length != 36)
                {
                    throw new Exception();
                }
                
                if (right[35] != right[34])
                {
                    throw new Exception();
                }
                
                if (right[34] != '_')
                {
                    throw new Exception();
                }
                
                if (right[15] != 'k')
                {
                    throw new Exception();
                }

                for (int i = 0; i < 15; ++i)
                {
                    if ((right[0 + i] ^ 58) != key[i])
                    {
                        throw new Exception();
                    }
                }

                return 30 + int.Parse(_calculator.Calculate(right.Substring(15, 19)));
            }
            catch (Exception e)
            {
                throw e;
            }
        }
    }
}
