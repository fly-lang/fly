namespace fly.os.time

public struct Time {
    long sec
    long nsec
}

public struct Duration {
    long nsec
}

public Time now()
public Time monotonic()
public sleep(Duration d)
public Duration since(Time t)
public Duration diff(Time a, Time b)
public Time add(Time t, Duration d)
public int compare(Time a, Time b)
public long unix(Time t)
public long unixNano(Time t)
public Time fromUnix(const long sec)
public Time fromUnixNano(const long nsec)
public string format(Time t, const string pattern)
public string formatSec(const long sec, const string pattern)
public Time parse(const string s, const string pattern)
public long parseToSec(const string s, const string pattern)
public long durationSecs(Duration d)
public long durationMillis(Duration d)
public long durationMicros(Duration d)
public string durationFormat(Duration d)
public long nowSec()
public string durationFormatNsec(const long nsec)
