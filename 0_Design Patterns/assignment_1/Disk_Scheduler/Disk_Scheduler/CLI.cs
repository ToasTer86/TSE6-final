using System;
using System.Threading;
using Disk_Scheduler_Strategies;
using Disk_Scheduler_Context;

namespace Disk_Scheduler
{
    class CLI
    {
        static ContextOS mainContext = new ContextOS();
        static void ShowHelp()
        {
            Console.WriteLine("Commands:");
            Console.WriteLine("-h: Show this help menu");
            Console.WriteLine("-s: Choose strategy");
            Console.WriteLine("-f: Find int at index");
            Console.WriteLine("-x: Exit Disk Scheduler");
        }

        static void ChooseStrategy()
        {
            Console.WriteLine("Please choose a strategy to use:");
            Console.WriteLine("1: First Come First Serve");
            Console.WriteLine("2: Shortest Seek Time First");
            var chosenStrategy = Console.ReadLine();
            Thread contextThread;
            IStrategy strategy;
            if (chosenStrategy == "1")
            {
                strategy = new FCFSStrategy();
                contextThread = new Thread(new ParameterizedThreadStart(mainContext.ChooseStrategy));
            }
            else if(chosenStrategy == "2")
            {
                strategy = new SSTFStrategy();
            }
            else
            {
                Console.WriteLine("Can not determine what strategy to use. Please try again");
                ChooseStrategy();
            }
             
        }

        public static int Main(string[] args)
        {
            var running = true;
            Console.WriteLine("Welcome to our Disk Scheduler.");
            ShowHelp();
            while (running)
            {
                Console.WriteLine("Please make your choice:");
                var choice = Console.ReadLine();
                switch (choice)
                {
                    case "-s":
                        Console.WriteLine("-s selected");
                        break;
                    case "-f":
                        Console.WriteLine("-f selected");
                        break;
                    case "-x":
                        running = false;
                        break;
                    case "-h":
                        ShowHelp();
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
