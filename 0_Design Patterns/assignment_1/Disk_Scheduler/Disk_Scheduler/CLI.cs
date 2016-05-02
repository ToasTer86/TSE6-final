using System;
using System.Threading;
using Disk_Scheduler_Context;
using Disk_Scheduler_Strategies;

namespace Disk_Scheduler
{
    class CLI
    {
        static readonly ContextOS MainContext = new ContextOS();
        private static void CliShowHelp()
        {
            Console.WriteLine("Commands:");
            Console.WriteLine("-h: Show this help menu");
            Console.WriteLine("-s: Choose strategy");
            Console.WriteLine("-f: Schedule search at index");
            Console.WriteLine("-g: Get next search result");
            Console.WriteLine("-x: Exit Disk Scheduler");
        }

        private static void CliChooseStrategy()
        {
            Console.WriteLine("Please choose a strategy to use:");
            Console.WriteLine("1: First Come First Serve");
            Console.WriteLine("2: Shortest Seek Time First");
            Console.WriteLine("3: Don't choose strategy");
            var chosenStrategy = Console.ReadLine();
            Thread contextThread;
            IStrategy strategy;
            switch (chosenStrategy)
            {
                case "1":
                    strategy = new FCFSStrategy();
                    contextThread = new Thread(MainContext.ChooseStrategy);
                    contextThread.Start(strategy);
                    break;
                case "2":
                    strategy = new SSTFStrategy();
                    contextThread = new Thread(MainContext.ChooseStrategy);
                    contextThread.Start(strategy);
                    break;
                case "3":
                    Console.WriteLine("No new strategy was chosen, returning to main menu");
                    break;
                default:
                    Console.WriteLine("Can not determine what strategy to use. Please try again");
                    CliChooseStrategy();
                    break;
            }
        }

        private static void CliScheduleSearch()
        {
            Console.WriteLine("At which index would you like to read?");
            Console.WriteLine("(input should be between 0-100)");
            var index = Console.ReadLine();
            int indexAsInt;
            var successfullyParsed = int.TryParse(index, out indexAsInt);
            if (successfullyParsed)
            {
                if (0 <= indexAsInt && indexAsInt <= 100)
                {
                    var contextThread = new Thread(MainContext.ScheduleSearch);
                    contextThread.Start(indexAsInt);
                }
                else
                {
                    Console.WriteLine("Input was not a number between 0-100. Please try again");
                    CliScheduleSearch();
                }
            }
            else
            {
                Console.WriteLine("Input could not be parsed as int. Please try again");
                CliScheduleSearch();
            }
        }

        static void CliGetNextResult()
        {
            Console.WriteLine("Fetching result...");
            var index = 0;
            var result = 0;
            var contextThread = new Thread(() => MainContext.ReturnNextResult(out index, out result));
            contextThread.Start();
            while (contextThread.IsAlive)
            {
                //Getting result
            }
            Console.WriteLine("At index {0}, value {1} was found", index, result);
        }

        public static int Main(string[] args)
        {
            var running = true;
            Console.WriteLine("Welcome to our Disk Scheduler.");
            CliShowHelp();
            while (running)
            {
                Console.WriteLine("Please make your choice:");
                var choice = Console.ReadLine();
                switch (choice)
                {
                    case "-s":
                        CliChooseStrategy();
                        break;
                    case "-f":
                        CliScheduleSearch();
                        break;
                    case "-x":
                        running = false;
                        break;
                    case "-h":
                        CliShowHelp();
                        break;
                    case "-g":
                        CliGetNextResult();
                        break;
                    default:
                        Console.WriteLine("Could not process input '{0}'. Try -h for help", choice);
                        break;
                }
            }
            Console.WriteLine("Leaving Disk Scheduler. Bye!");
            return 0;
        }
    }
}
