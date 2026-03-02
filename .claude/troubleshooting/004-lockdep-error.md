# lockdep.c 编译错误

## 问题描述
内核编译时 lockdep.c 报错

## 错误信息
```
kernel/locking/lockdep.c:4008:27: error: use of undeclared identifier 'nested'
1 error generated.
```

## 原因分析
内核源码 bug：nested 参数已从 lock_release() 函数移除，但调用处未更新

## 解决方案
修改 kernel/locking/lockdep.c，将 nested 改为 0

## 修复文件
- `kernel/locking/lockdep.c` - 将 `if (__lock_release(lock, nested, ip))` 改为 `if (__lock_release(lock, 0, ip))`
