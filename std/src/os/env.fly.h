namespace fly.os.env

public struct StringArray {
    long items
    int count
}

public get(const string key, string out)
public set(const string key, const string value)
public delete(const string key)
public all(StringArray out)
public expand(const string s, string out)
public cwdGet(string out)
public cwdSet(const string path)
public argsGet(StringArray out)
public argsCount(int out)
public hostname(string out)
public osname(string out)
public exit(const int code)
public allCount(int out)
