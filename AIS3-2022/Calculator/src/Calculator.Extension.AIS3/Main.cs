using System;
using System.ComponentModel.Composition;

namespace Calculator.Extension.AIS3
{
    [Export(typeof(IOperation))]
    [ExportMetadata("Symbol", 'A')]
    public class AIS3 : IOperation
    {
        private ICalculator _calculator;

        [ImportingConstructor]
        public AIS3(ICalculator calculator)
        {
            _calculator = calculator;
        }

        public int Operate(string left, string right)
        {
            try
            {
                int[] key = {30, 4, 100};

                if (right.Length != 45)
                {
                    throw new Exception();
                }
                
                if (right[14] != 'A')
                {
                    throw new Exception();
                } 
                
                if (right[3] != '{')
                {
                    throw new Exception();
                }

                for (int i = 0; i < 3; ++i)
                {
                    if((right[i] ^ 87) != key[i])
                    {
                        throw new Exception();
                    }
                }

                return 1000 + int.Parse(_calculator.Calculate(right.Substring(3)));
            }
            catch (Exception e)
            {
                throw e;
            }
        }
    }
}