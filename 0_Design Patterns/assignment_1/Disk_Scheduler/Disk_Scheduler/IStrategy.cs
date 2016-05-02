using System.Collections.Generic;
using Disk_Scheduler_SearchResult;

namespace Disk_Scheduler_Strategies
{
    interface IStrategy
    {
        SearchResult GetDataFromIndex(List<int> index, List<int> data);
    }
}