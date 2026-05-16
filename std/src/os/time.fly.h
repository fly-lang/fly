namespace fly.os.time

public struct Time {
    long sec
    long nsec
}

public struct Duration {
    long nsec
}

public now(Time out)
public monotonic(Time out)
public sleep(Duration d)
public since(Time t, Duration out)
public diff(Time a, Time b, Duration out)
public add(Time t, Duration d, Time out)
public compare(Time a, Time b, int out)
public unix(Time t, long out)
public unixNano(Time t, long out)
public fromUnix(const long sec, Time out)
public fromUnixNano(const long nsec, Time out)
public format(Time t, const string pattern, string out)
public formatSec(const long sec, const string pattern, string out)
public parse(const string s, const string pattern, Time out)
public parseToSec(const string s, const string pattern, long out)
public durationSecs(Duration d, long out)
public durationMillis(Duration d, long out)
public durationMicros(Duration d, long out)
public durationFormat(Duration d, string out)
public nowSec(long out)
public durationFormatNsec(const long nsec, string out)
