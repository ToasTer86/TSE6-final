using System.Collections.Generic;
using Disk_Scheduler_SearchResult;

namespace Disk_Scheduler_Strategies
{
    class FCFSStrategy : IStrategy
    {
        public SearchResult GetDataFromIndex(List<int> index, List<int> data)
        {
            if (index.Count > 0)
            {
                var indexToSearchAt = index[0];
                var valueFound = data[indexToSearchAt];
                return new SearchResult(indexToSearchAt, valueFound);
            }
            return new SearchResult(-1, -2);
        }
    }
}
