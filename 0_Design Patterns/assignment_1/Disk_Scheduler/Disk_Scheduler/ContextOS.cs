using System;
using System.Collections.Generic;
using Disk_Scheduler_Strategies;

namespace Disk_Scheduler_Context
{
    class ContextOS
    {
        private IStrategy strategy = null;
        private List<int> scheduledSearches = new List<int>();
        private List<int> data = new List<int>();

        public ContextOS()
        {
            var random = new Random();
            for (var i = 0; i < 100; i++)
            {
                //var randomValue = random.Next(0, 100);
                var randomValue = i*2;
                data.Add(randomValue);
            }
        }

        public void ChooseStrategy(object inputStrategy)
        {
            var givenStrategy = inputStrategy as IStrategy;
            if (givenStrategy != null)
            {
                strategy = givenStrategy;
            }
        }

        public void ReturnNextResult(out int index, out int result)
        {
            index = -1;
            result = -1;
            if (strategy != null)
            {
                var searchResult = strategy.GetDataFromIndex(scheduledSearches, data);
                index = searchResult.IndexOfResult;
                result = searchResult.ValueOfResult;
                scheduledSearches.Remove(index);
            }
        }

        public void ScheduleSearch(object index)
        {
            var locationToSearch = index as int?;
            if (locationToSearch != null)
            {
                scheduledSearches.Add(locationToSearch.Value);
            }
        }
    }
}
