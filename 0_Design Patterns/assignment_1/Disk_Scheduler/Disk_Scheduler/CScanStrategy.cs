using System.Collections.Generic;
using System.Linq;
using Disk_Scheduler_SearchResult;

namespace Disk_Scheduler_Strategies
{
    class CScanStrategy : IStrategy
    {
        private int lastIndex = 0;
        public SearchResult GetDataFromIndex(List<int> index, List<int> data)
        {
            if (index.Count > 0)
            {
                var closestIndex = GetClosestIndex(index);
                if (closestIndex == 0)
                {
                    lastIndex = 0;
                    closestIndex = GetClosestIndex(index);
                }
                lastIndex = closestIndex;
                return new SearchResult(lastIndex, data[lastIndex]);
            }
            return new SearchResult(-1, -2);
        }

        private int GetClosestIndex(List<int> indexes)
        {
            int shortestDistance = indexes.Max();
            int returnIndex = 0;
            foreach (var i in indexes)
            {
                if (i >= lastIndex)
                {
                    if ((i - lastIndex) <= shortestDistance)
                    {
                        shortestDistance = (i - lastIndex);
                        returnIndex = i;
                    }
                }
            }
            return returnIndex;
        }
    }
}
