using System;
using System.Collections.Generic;
using Disk_Scheduler_SearchResult;

namespace Disk_Scheduler_Strategies
{
    class SSTFStrategy : IStrategy
    {
        private int currentIndex = 0;
        public SearchResult GetDataFromIndex(List<int> index, List<int> data)
        {
            if (index.Count > 0)
            {
                var closestIndex = 0;
                var closestDistance = data.Count;
                foreach (var i in index)
                {
                    var distance = Math.Abs(i - currentIndex);
                    if (distance < closestDistance)
                    {
                        closestDistance = distance;
                        closestIndex = i;
                    }
                }
                currentIndex = closestIndex;
                return new SearchResult(currentIndex, data[currentIndex]);
            }
            return new SearchResult(-1, -2);
        }
    }
}
