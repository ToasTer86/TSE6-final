using Disk_Scheduler_Strategies;

namespace Disk_Scheduler_Context
{
    class ContextOS
    {
        private IStrategy strategy;

        public ContextOS()
        {
            //fill list
        }

        public void ChooseStrategy(IStrategy strategy)
        {
            this.strategy = strategy;
        }

        public void ReturnLastResult(out int index, out int result)
        {
            index = 1;
            result = 1;
        }

        public bool PerformSearch(int index)
        {
            return false;
        }
    }
}
