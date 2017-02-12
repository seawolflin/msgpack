# msgpack
 msgpack是一个用C语言实现的MessagePack协议。实现的是5版本的[MessagePack Specification](https://github.com/msgpack/msgpack/blob/master/spec.md)。
 msgpack的实现目标是footprint最小，快速的流式编码解码，且无存额内存开销。可使用在footprint受限以及内存受限的嵌入式环境中。

 **注：**
 目前尚未实现ext类型。

## 许可协议
  msgpack的使用由MIT授权。

## 例子
```
  msgpack_buffer_t mbuf;
  int8_t data[64];

  init_msgpack_buffer(&mbuf, data, 64);
  /* Encode map ["name":"Joan", "age": 30, "gender", "female"] */
  msgpack_write_map(&mbuf, 3);
  msgpack_write_str(&mbuf, "name", strlen("name"));
  msgpack_write_str(&mbuf, "Joan", strlen("Joan"));
  msgpack_write_str(&mbuf, "age", strlen("age"));
  msgpack_write_integer(&mbuf, 30);
  msgpack_write_str(&mbuf, "gender", strlen("gender"));
  msgpack_write_str(&mbuf, "female", strlen("female"));

  msgpack_unpacker_t unpacker;
  uint8_t type;
  uint32_t len;
  uint32_t map_len;
  char key[8] = {0};
  char strval[8] = {0};
  int age = 0;
  int i;
  init_msgpack_unpacker(&unpacker, mbuf.buf, mbuf.len, 0);
  type = MSGPACK_TYPE_ANY;
  msgpack_unpack(&unpacker, NULL, &map_len, &type);
  if (msgpack_errno() == MSGPACK_EOK) {
    printf("map len: %d \n[", map_len);
  }

  for (i = 0; i < map_len; ++i) {
    type = MSGPACK_TYPE_ANY;
    len = 8;
    msgpack_unpack(&unpacker, &key, &len, &type);
    if (msgpack_errno() == MSGPACK_EOK) {
      key[len]=0;
      printf("\"%s\":", key);
    }
    type = MSGPACK_TYPE_ANY;
    if (i == 1) {
      len = 4;
      msgpack_unpack(&unpacker, &age, &len, &type);
      if (msgpack_errno() == MSGPACK_EOK) {
        printf("%d", age);
      }
    } else {
      len = 8;
      msgpack_unpack(&unpacker, &strval, &len, &type);
      if (msgpack_errno() == MSGPACK_EOK) {
        strval[len] = 0;
        printf("\"%s\"", strval);
      }
    }
    if (i < map_len - 1) {
      printf(", ");
    }
  }
  printf("]\n");
```

