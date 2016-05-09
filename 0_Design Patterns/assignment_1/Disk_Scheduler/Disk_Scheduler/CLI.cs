using System;
using System.Threading;
using Disk_Scheduler_Context;
using Disk_Scheduler_Strategies;

namespace Disk_Scheduler
{
    class CLI
    {
        private static readonly ContextOS MainContext = new ContextOS();

        private static void AddSearchRequest(int indexToAdd)
        {
            var contextThread = new Thread(MainContext.ScheduleSearch);
            contextThread.Start(indexToAdd);
            while (contextThread.IsAlive) {; }
        }

        private static void SetFCFSStrategy()
        {
            IStrategy strategy = new FCFSStrategy();
            var contextThread = new Thread(MainContext.ChooseStrategy);
            contextThread.Start(strategy);
        }

        private static void SetSSTFStrategy()
        {
            IStrategy strategy = new SSTFStrategy();
            var contextThread = new Thread(MainContext.ChooseStrategy);
            contextThread.Start(strategy);
        }

        private static void SetCScanStrategy()
        {
            IStrategy strategy = new CScanStrategy();
            var contextThread = new Thread(MainContext.ChooseStrategy);
            contextThread.Start(strategy);
        }

        private static void CliShowHelp()
        {
            Console.WriteLine("Commands:");
            Console.WriteLine("-h: Show this help menu");
            Console.WriteLine("-s: Choose strategy");
            Console.WriteLine("-f: Schedule search at index");
            Console.WriteLine("-g: Get next search result");
            Console.WriteLine("-x: Exit Disk Scheduler");
            Console.WriteLine("-fd: First Come First Serve Demo");
            Console.WriteLine("-sd: Shortest Seek Time First Demo");
            Console.WriteLine("-cd: Circular Scan Demo");
        }

        private static void CliChooseStrategy()
        {
            Console.WriteLine("Please choose a strategy to use:");
            Console.WriteLine("1: First Come First Serve");
            Console.WriteLine("2: Shortest Seek Time First");
            Console.WriteLine("3: Circular Scan");
            Console.WriteLine("4: Don't choose strategy");
            Console.WriteLine("Please make your choice:");
            var chosenStrategy = Console.ReadLine();
            switch (chosenStrategy)
            {
                case "1":
                    SetFCFSStrategy();
                    break;
                case "2":
                    SetSSTFStrategy();
                    break;
                case "3":
                    SetCScanStrategy();
                    break;
                case "4":
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
                    AddSearchRequest(indexAsInt);
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
            if ((index == -1) && (result == -1))
            {
                Console.WriteLine("No strategy has been chosen yet. Please choose a strategy to perform search requests");
            }
            else if ((index == -1) && (result != -1))
            {
                Console.WriteLine("No search requests are queued. Please provide an index to search at.");
            }
            else
            {
                Console.WriteLine("At index {0}, value {1} was found", index, result);
            }
        }

        static void CliPerformFCFSDemo() 
        {
            Console.WriteLine("Starting First Come First Serve demo.");
            Console.WriteLine("Filling search queries");
            AddSearchRequest(5);
            AddSearchRequest(25);
            AddSearchRequest(10);
            AddSearchRequest(50);
            AddSearchRequest(75);
            Console.WriteLine("Search queries are: 5, 25, 10, 50, 75");
            SetFCFSStrategy();
            Console.WriteLine("Performing searches...");
            for (var i = 0; i < 5; i++)
            {
                CliGetNextResult();
            }
            Console.WriteLine("First Come First Serve demo finished!");
        }

        static void CliPerformSSTFDemo()
        {
            Console.WriteLine("Starting Shortest Seek Time First demo.");
            Console.WriteLine("Filling search queries");
            AddSearchRequest(5);
            AddSearchRequest(25);
            AddSearchRequest(10);
            AddSearchRequest(50);
            AddSearchRequest(75);
            AddSearchRequest(51);
            AddSearchRequest(55);
            AddSearchRequest(51);
            AddSearchRequest(55);
            AddSearchRequest(51);
            AddSearchRequest(55);
            AddSearchRequest(51);
            AddSearchRequest(55);
            Console.WriteLine("Search queries are: 5, 25, 10, 50, 75, 51, 55, 51, 55, 51, 55, 51, 55");
            Console.WriteLine("With this set of search requests, starvation can be seen (case 75)");
            SetSSTFStrategy();
            Console.WriteLine("Performing searches...");
            for (var i = 0; i < 13; i++)
            {
                CliGetNextResult();
            }
            Console.WriteLine("Shortest Seek Time First demo finished!");
        }

        static void CliPerformCScanDemo()
        {
            Console.WriteLine("Starting Circular Scan demo.");
            SetCScanStrategy();
            Console.WriteLine("Filling search queries");
            Console.WriteLine("Search queries are: 5, 25, 1");
            AddSearchRequest(5);
            CliGetNextResult();
            AddSearchRequest(1);
            AddSearchRequest(25);
            CliGetNextResult();
            CliGetNextResult();
            Console.WriteLine("Circular Scan demo finished!");
        }

        public static int Main(string[] args)
        {
            var running = true;
            Console.WriteLine("Welcome to our Disk Scheduler.");
            CliShowHelp();
            while (running)
            {
                Console.WriteLine("Please make your choice: (-h for help)");
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
                    case "-fd":
                        CliPerformFCFSDemo();
                        break;
                    case "-sd":
                        CliPerformSSTFDemo();
                        break;
                    case "-cd":
                        CliPerformCScanDemo();
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
