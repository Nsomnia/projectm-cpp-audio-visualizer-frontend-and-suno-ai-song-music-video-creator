#!/bin/env bash
#curl -s https://suno.com/song/a03cc5e1-d96d-4204-bdf0-997f3464bc4b | sed -n 's/.*18:Tc0c,\"\\\(.*\\\)\\\"])\\<\\/script\\>.*/\\1/p' | sed 's/\\n/\n/g' | sed 's/\\\"/\"/g'
#curl -s https://suno.com/song/a03cc5e1-d96d-4204-bdf0-997f3464bc4b | grep -oP '18:Tc0c,\"\\K.*?(?=\\\"]\\)\\<\\/script\\>)' | sed 's/^])<\/script><script>self.__next_f.push(\[1,"//' | sed 's/\\\"/\"/g' | xargs -0 echo -e

curl -s https://suno.com/song/a03cc5e1-d96d-4204-bdf0-997f3464bc4b | grep '18:Tc0c,' | sed -n 's/.*18:Tc0c,\"\(.*\)\"])\)<\/script>.*/\1/p' | sed 's/\\n/\n/g' | sed 's/\\\"/\"/g'