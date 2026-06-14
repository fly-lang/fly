namespace fly.os.env

public struct StringArray {
    long items
    int count
}

public string get(const string key)
public set(const string key, const string value)
public delete(const string key)
public StringArray all()
public string expand(const string s)
public string cwdGet()
public cwdSet(const string path)
public StringArray argsGet()
public int argsCount()
public string hostname()
public string osname()
public exit(const int code)
public int allCount()
