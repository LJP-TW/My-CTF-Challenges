using System;
using System.Collections.Generic;
using System.Linq;
using System.ComponentModel.Composition;

namespace Calculator
{
    [Export(typeof(ICalculator))]
    public class DefaultCalculator : ICalculator
    {
        [ImportMany]
        IEnumerable<Lazy<IOperation, IOperationMetadata>> operations;

        public string Calculate(string expression)
        {
            string left, right;
            char operation;
            int foundOperatorIndex;
            
            foundOperatorIndex = FindFirstNonDigit(expression);

            if (foundOperatorIndex < 0)
            {
                return "Invalid expression A_A";
            }

            try
            {
                left = expression.Substring(0, foundOperatorIndex);
                right = expression.Substring(foundOperatorIndex + 1);
            }
            catch (Exception)
            {
                return "Invalid expression G_G";
            }

            operation = expression[foundOperatorIndex];

            var matchOP = operations.Where((op) => op.Metadata.Symbol.Equals(operation)).FirstOrDefault();

            if (matchOP == null)
            {
                return "Invalid expression Q_Q";
            }

            try
            {
                return matchOP.Value.Operate(left, right).ToString();
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