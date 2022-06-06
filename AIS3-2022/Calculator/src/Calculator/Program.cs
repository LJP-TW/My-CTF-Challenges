using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Hosting;

namespace Calculator
{
    class Program
    {
        private CompositionContainer _container;

        [Import]
        public ICalculator calculator;

        public Program()
        {
            var catalog = new AggregateCatalog();
            string extensionPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Extensions");

            Console.WriteLine(extensionPath);

            catalog.Catalogs.Add(new AssemblyCatalog(typeof(DefaultCalculator).Assembly));
            catalog.Catalogs.Add(new DirectoryCatalog(extensionPath, "*.dll"));

            _container = new CompositionContainer(catalog);
            _container.ComposeParts(this);
        }

        static void Main(string[] args)
        {
            Program p = new Program();
            String s;

            if (p.calculator == null)
            {
                return;
            }

            Console.WriteLine("Simple calculator (there are many bugs G_G)");
            Console.WriteLine("Example: 2*3+4-5");
            Console.WriteLine("> ");
            while (true)
            {
                s = Console.ReadLine();
                try
                {
                    Console.WriteLine(p.calculator.Calculate(s));
                }
                catch (Exception)
                {
                    Console.WriteLine("Invalid expression ?_?");
                }
            }
        }
    }
}