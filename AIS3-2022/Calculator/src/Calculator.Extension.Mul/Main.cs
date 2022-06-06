using System;
using System.ComponentModel.Composition;

namespace Calculator.Extension.Mul
{
    [Export(typeof(IOperation))]
    [ExportMetadata("Symbol", '*')]
    public class Mul : IOperation
    {
        private ICalculator _calculator;

        [ImportingConstructor]
        public Mul(ICalculator calculator)
        {
            _calculator = calculator;
        }
        public int Operate(string left, string right)
        {
            try
            {
                int foundOperatorIndex;

                foundOperatorIndex = FindFirstNonDigit(right);

                if (foundOperatorIndex == -1)
                    return int.Parse(left) * int.Parse(right);

                return int.Parse(
                    _calculator.Calculate(
                        (int.Parse(left) * int.Parse(right.Substring(0, foundOperatorIndex))).ToString() +
                        right.Substring(foundOperatorIndex)
                        ));
            }
            catch (Exception e)
            {
                throw e;
            }
        }

        private int FindFirstNonDigit(string str)
        {
            for (int i = 0; i < str.Length; ++i)
            {
                if (!char.IsDigit(str[i]))
                {
                    return i;
                }
            }

            return -1;
        }
    }
}