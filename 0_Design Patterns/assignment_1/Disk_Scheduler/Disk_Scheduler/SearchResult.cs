namespace Disk_Scheduler_SearchResult
{
    class SearchResult
    {
        public SearchResult(int indexOfResult, int valueOfResult)
        {
            IndexOfResult = indexOfResult;
            ValueOfResult = valueOfResult;
        }
        public int IndexOfResult { get; }

        public int ValueOfResult { get; }
    }
}
