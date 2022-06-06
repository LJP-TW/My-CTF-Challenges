namespace Calculator
{
    public interface IOperation
    {
        int Operate(string left, string right);
    }

    public interface IOperationMetadata
    {
        char Symbol { get; }
    }
}
