//   Copyright 2022 bindis - caozhanhao
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
#include "libczh/include/libczh/czh.hpp"
#include <iostream>
#include <random>
#include <thread>
#include <atomic>

namespace bindis
{
  int get_random(double probability)
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> dis({probability, 1 - probability});
    return dis(gen);
  }
  
  double bindis(int count, int max, int min, int nsamples, double probability, int nthreads)
  {
    std::atomic<int> frequency = 0;
    auto task = [&frequency, count, probability, min, max]
    {
      int head = 0;
      for (int j = 0; j < count; ++j)
      {
        if (get_random(probability)) head++;
      }
      if (head >= min && head <= max) frequency++;
    };
    std::vector<std::thread> threads;
    threads.reserve(static_cast<size_t>(nthreads));
    int average = nsamples / nthreads;
    for (int i = 0; i < nthreads - 1; ++i)
    {
      threads.emplace_back([i, average, &task]()
                           {
                             for (int j = i * average; j < (i + 1) * average; j++)
                             {
                               task();
                             }
                           });
    }
    threads.emplace_back([average, &task, nthreads, nsamples]()
                         {
                           for (int j = (nthreads - 1) * average; j < nsamples; j++)
                           {
                             task();
                           }
                         });
    
    for (auto &t: threads)
    {
      t.join();
    }
    return static_cast<double>(frequency) / static_cast<double>(nsamples);
  }
}

int main()
{
  auto czhptr = czh::Czh("config.czh", czh::InputMode::nonstream).parse();
  auto &config = (*czhptr)["config"];
  double result = bindis::bindis(
      config["count"].get<int>(),
      config["max"].get<int>(),
      config["min"].get<int>(),
      config["nsamples"].get<int>(),
      config["probability"].get<double>(),
      config["nthreads"].get<int>()
  );
  std::cout << "The frequency is "
            << result * 100
            << "%." << std::endl;
  std::fstream output_file("config.czh", std::ios_base::out);
  (*czhptr)["result"]["last_frequency"] = double(result);
  output_file << *czhptr;
  return 0;
}