using System;
using System.ComponentModel.Composition;

namespace Calculator.Extension.AIS33
{
    [Export(typeof(IOperation))]
    [ExportMetadata("Symbol", '{')]
    public class AIS33 : IOperation
    {
        private ICalculator _calculator;

        [ImportingConstructor]
        public AIS33(ICalculator calculator)
        {
            _calculator = calculator;
        }

        public int Operate(string left, string right)
        {
            try
            {
                int[] key = { 5, 29, 5 };

                if (right.Length != 41)
                {
                    throw new Exception();
                }
               
                if (right[40] != '}')
                {
                    throw new Exception();
                }
                
                if (right[0] != 'D')
                {
                    throw new Exception();
                }

                for (int i = 0; i < 3; ++i)
                {
                    if ((right[37 + i] ^ 66) != key[i])
                    {
                        throw new Exception();
                    }
                }

                return 300 + int.Parse(_calculator.Calculate(right.Substring(0, 37)));
            }
            catch (Exception e)
            {
                throw e;
            }
        }
    }
}
