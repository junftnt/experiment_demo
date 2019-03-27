#include <stdio.h>

int main() {

    int nums[10] = {2,5,7,1,4,9,6,0,8,3};
    int len = 10, tmp;

    /*
    // 冒泡
    for (int i = 0; i < len-1; i++) {
        for (int j = i+1; j < len; j++) {
            if (nums[i] < nums[j]) {
                tmp = nums[i];
                nums[i] = nums[j];
                nums[j] = tmp;
            }
        }
    }

    // 简单选择排序，跟冒泡的区别就是找到最小的以后再调换
    int tmp_key;
    for (int i = 0; i < len-1; i++) {
        tmp = 0;
        for (int j = i+1; j < len; j++) {
            if (tmp < nums[j]) {
                tmp = nums[j];
                tmp_key = j;
            }
        }
        if (tmp != i)
            nums[tmp_key] = nums[i];
            nums[i] = tmp;
    }

    // 直接插入排序
    int j, k;
    for(int i = 1; i < len; i ++) {
        if (nums[i] < nums[i-1]) {
            // 找到插入的点
            for (j=i-1; j >= 0; j--) {
                if (nums[j] < nums[i]) {
                    break;
                }
            }
            // 移动数组
            tmp = nums[i];
            for (k = i - 1; k > j; k--) {
                nums[k+1] = nums[k];
            }
            nums[k+1] = tmp;
        }
    }

    // 希尔排序，采用了 跳跃的方式，将数组依次拆分成 4,2,1 跨度的小数组进行直接插入排序
    int i, j, increment = len;
    do {
        increment = increment / 3 + 1;
        for (i = increment; i < len; i ++) {
            if (nums[i] < nums[i - increment]) {
                tmp = nums[i];
                for (j = i - increment; j >= 0 && tmp < nums[j]; j -= increment) {
                    nums[j+increment] = nums[j];
                }
                nums[j + increment] = tmp;
            }
        }
    } while (increment > 1);
    */

    // 堆排序













    for (int z = 0; z < 10; z++) {
        printf("%d \r\n", nums[z]);
    }
    return 0;
}
