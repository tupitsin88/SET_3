#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>
class ArrayGenerator {
private:
    std::mt19937 rng;
    static const int kMinVal = 0;
    static const int kMaxVal = 6000;

public:
    explicit ArrayGenerator(unsigned seed = 42) : rng(seed) {
    }

    // Случайный массив
    std::vector<int> GenerateRandom(int size) {
        std::uniform_int_distribution<int> dist(kMinVal, kMaxVal);
        std::vector<int> arr;
        for (int i = 0; i < size; i++) {
            arr.push_back(dist(rng));
        }
        return arr;
    }

    // Массив в обратном порядке (от max к min)
    std::vector<int> GenerateReverse(int size) {
        std::vector<int> arr(size);
        for (int i = 0; i < size; ++i) {
            arr[i] = kMaxVal - i * kMaxVal / size;
        }
        return arr;
    }

    // "Почти отсортированный" массив
    // Берём отсортированный, делаем несколько обменов
    std::vector<int> GenerateNearlySorted(int size, double swap_ratio = 0.05) {
        std::vector<int> arr(size);
        for (int i = 0; i < size; ++i) {
            arr[i] = kMinVal + i * kMaxVal / size;
        }
        int num_swaps = static_cast<int>(size * swap_ratio);
        std::uniform_int_distribution<int> dist(0, size - 1);
        for (int i = 0; i < num_swaps; ++i) {
            int idx1 = dist(rng);
            int idx2 = dist(rng);
            std::swap(arr[idx1], arr[idx2]);
        }
        return arr;
    }
};

// Стандартный MERGE SORT (рекурсивный)
class StandardMergeSort {
public:
    static void Sort(std::vector<int> &arr) {
        if (arr.empty()) {
            return;
        }
        MergeSort(arr, 0, static_cast<int>(arr.size()) - 1);
    }

private:
    static void MergeSort(std::vector<int> &arr, int left, int right) {
        if (left < right) {
            int mid = left + (right - left) / 2;
            MergeSort(arr, left, mid);
            MergeSort(arr, mid + 1, right);
            Merge(arr, left, mid, right);
        }
    }

    static void Merge(std::vector<int> &arr, int left, int mid, int right) {
        std::vector<int> temp(right - left + 1);
        int i = left;
        int j = mid + 1;
        int k = 0;

        while (i <= mid && j <= right) {
            if (arr[i] <= arr[j]) {
                temp[k++] = arr[i++];
            } else {
                temp[k++] = arr[j++];
            }
        }

        while (i <= mid) {
            temp[k++] = arr[i++];
        }
        while (j <= right) {
            temp[k++] = arr[j++];
        }

        for (i = left, k = 0; i <= right; ++i, ++k) {
            arr[i] = temp[k];
        }
    }
};

// MERGE + INSERTION SORT
class HybridMergeSort {
private:
    int threshold_;

public:
    explicit HybridMergeSort(int threshold = 42) : threshold_(threshold) {
    }

    void Sort(std::vector<int> &arr) {
        if (arr.empty()) {
            return;
        }
        MergeSort(arr, 0, static_cast<int>(arr.size()) - 1);
    }

private:
    static void InsertionSort(std::vector<int> &arr, int left, int right) {
        for (int i = left + 1; i <= right; ++i) {
            int key = arr[i];
            int j = i - 1;

            while (j >= left && arr[j] > key) {
                arr[j + 1] = arr[j];
                j--;
            }
            arr[j + 1] = key;
        }
    }

    void MergeSort(std::vector<int> &arr, int left, int right) {
        if (right - left + 1 <= threshold_) {
            InsertionSort(arr, left, right);
        } else if (left < right) {
            int mid = left + (right - left) / 2;
            MergeSort(arr, left, mid);
            MergeSort(arr, mid + 1, right);
            Merge(arr, left, mid, right);
        }
    }

    static void Merge(std::vector<int> &arr, int left, int mid, int right) {
        std::vector<int> temp(right - left + 1);
        int i = left;
        int j = mid + 1;
        int k = 0;

        while (i <= mid && j <= right) {
            if (arr[i] <= arr[j]) {
                temp[k++] = arr[i++];
            } else {
                temp[k++] = arr[j++];
            }
        }

        while (i <= mid) {
            temp[k++] = arr[i++];
        }
        while (j <= right) {
            temp[k++] = arr[j++];
        }

        for (i = left, k = 0; i <= right; ++i, ++k) {
            arr[i] = temp[k];
        }
    }
};

struct BenchmarkResult {
    int size;
    long long avg_time; // в миллисекундах
};

class SortTester {
private:
    static const int NUM_RUNS = 10;
public:
    // Замеры для стандартного MERGE SORT
    static std::vector<BenchmarkResult> BenchmarkStandardMerge(
        const std::vector<std::vector<int> > &test_arrays) {
        std::vector<BenchmarkResult> results;

        for (const auto &arr: test_arrays) {
            long long total_time = 0;

            for (int run = 0; run < NUM_RUNS; ++run) {
                std::vector<int> temp = arr; // копируем массив

                auto start = std::chrono::high_resolution_clock::now();
                StandardMergeSort::Sort(temp);
                auto elapsed = std::chrono::high_resolution_clock::now() - start;

                auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
                total_time += msec;
            }
            auto avg_time = total_time / NUM_RUNS;
            results.push_back({static_cast<int>(arr.size()), avg_time});
        }
        return results;
    }
    // Замеры для гибридного MERGE+INSERTION SORT
    static std::vector<BenchmarkResult> BenchmarkHybridMerge(
        const std::vector<std::vector<int> > &test_arrays,
        int threshold) {
        std::vector<BenchmarkResult> results;
        HybridMergeSort sorter(threshold);

        for (const auto &arr: test_arrays) {
            long long total_time = 0;

            for (int run = 0; run < NUM_RUNS; ++run) {
                std::vector<int> temp = arr; // копируем массив

                auto start = std::chrono::high_resolution_clock::now();
                sorter.Sort(temp);
                auto elapsed = std::chrono::high_resolution_clock::now() - start;

                auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
                total_time += msec;
            }
            auto avg_time = total_time / NUM_RUNS;
            results.push_back({static_cast<int>(arr.size()), avg_time});
        }
        return results;
    }
};

int main() {
    ArrayGenerator gen(12345);
    const int MIN_SIZE = 500;
    const int MAX_SIZE = 100000;
    const int STEP = 100;
    // Пороги для гибридного алгоритма
    std::vector<int> thresholds = {10, 20, 30, 40, 50};
    std::vector<std::string> data_types = {"random", "reverse", "nearly_sorted"};
    for (const auto &data_type: data_types) {
        std::vector<int> max_array;
        if (data_type == "random") {
            max_array = gen.GenerateRandom(MAX_SIZE);
        } else if (data_type == "reverse") {
            max_array = gen.GenerateReverse(MAX_SIZE);
        } else {
            max_array = gen.GenerateNearlySorted(MAX_SIZE, 0.05);
        }
        std::vector<std::vector<int> > test_arrays;
        for (int size = MIN_SIZE; size <= MAX_SIZE; size += STEP) {
            std::vector<int> subarray(max_array.begin(), max_array.begin() + size);
            test_arrays.push_back(subarray);
        }
        // ========== StandardMergeSort ==========
        std::cout << "\n Тестирование StandardMergeSort " << std::endl;
        auto results_merge = SortTester::BenchmarkStandardMerge(test_arrays);
        std::string filename_merge = "results_merge_" + data_type + ".csv";
        std::ofstream out_merge(filename_merge);
        if (!out_merge.is_open()) {
            std::cerr << "Ошибка: не могу открыть файл " << filename_merge << std::endl;
            continue;
        }
        out_merge << "size,time_ms\n";
        for (const auto &result: results_merge) {
            out_merge << result.size << "," << result.avg_time << "\n";
            std::cout << "Size: " << std::setw(6) << result.size
                    << " | Avg time: " << std::setw(6) << result.avg_time << " ms" << std::endl;
        }
        out_merge.flush();
        out_merge.close();
        std::cout << " Сохранено в " << filename_merge << std::endl;
        // ========== HybridMergeSort ==========
        std::cout << "\n Тестирование HybridMergeSort " << std::endl;
        for (int threshold: thresholds) {
            std::cout << "\nПорог переключения: " << threshold << std::endl;
            auto results_hybrid = SortTester::BenchmarkHybridMerge(test_arrays, threshold);
            std::string filename_hybrid = "results_hybrid_" + data_type + "_th" +
                                         std::to_string(threshold) + ".csv";
            std::ofstream out_hybrid(filename_hybrid);
            if (!out_hybrid.is_open()) {
                std::cerr << "Ошибка: невозможно открыть файл " << filename_hybrid << std::endl;
                continue;
            }
            out_hybrid << "size,time_ms\n";
            for (const auto &result: results_hybrid) {
                out_hybrid << result.size << "," << result.avg_time << "\n";
                std::cout << "  Size: " << std::setw(6) << result.size
                        << " | Avg time: " << std::setw(6) << result.avg_time << " ms" << std::endl;
            }
            out_hybrid.flush();
            out_hybrid.close();
            std::cout << "  Сохранено в " << filename_hybrid << std::endl;
        }
    }
    std::cout << "\n Тестирование завершено " << std::endl;
    std::cout << "CSV файлы сохранены в текущей директории" << std::endl;
    return 0;
}
