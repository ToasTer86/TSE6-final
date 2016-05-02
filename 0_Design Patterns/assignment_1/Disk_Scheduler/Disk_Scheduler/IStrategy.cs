using System.Collections.Generic;

namespace Disk_Scheduler_Strategies
{
    interface IStrategy
    {
        int GetDataFromIndex(List<int> index, List<int> data);
    }
}