# Token Table

保存所有 token 的内容和位置信息。

```c++
struct Token {
    const std::string text;
    const int first_line, first_colum;
    const int last_line, last_colum;
    std::string dump() const;
}

std::vector<Token> token_table;
typedef int location;	// token_table[location]
```

