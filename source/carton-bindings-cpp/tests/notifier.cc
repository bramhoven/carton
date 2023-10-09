// Copyright 2023 Vivek Panyam
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cassert>
#include <iostream>

#include "../src/carton.hh"

int main()
{
    using carton::AsyncNotifier;
    using carton::Carton;
    using carton::DataType;
    using carton::Tensor;
    using carton::TensorMap;

    AsyncNotifier<Carton> load_notifier;
    AsyncNotifier<TensorMap> infer_notifier;

    // Load a model
    Carton::load("https://carton.pub/google-research/bert-base-uncased", load_notifier.handle(), (void *)42);

    // Wait for it to load
    auto [model_result, arg] = load_notifier.wait();
    assert(arg == (void *)42);
    auto model = model_result.get_or_throw();

    uint64_t shape[]{1};
    auto tensor = Tensor(DataType::kString, shape);
    tensor.set_string(0, "Today is a good [MASK].");

    std::unordered_map<std::string, Tensor> inputs;
    inputs.insert(std::make_pair("input", std::move(tensor)));

    // Run inference
    model.infer(std::move(inputs), infer_notifier.handle(), (void *)11);

    // Wait for inference to complete
    auto [infer_result, infer_arg] = infer_notifier.wait();
    assert(infer_arg == (void *)11);
    auto out = infer_result.get_or_throw();

    const auto tokens = out.get_and_remove("tokens");
    const auto scores = out.get_and_remove("scores");

    const auto scores_data = static_cast<const float *>(scores.data());

    std::cout << "Got output token: " << tokens.get_string(0) << std::endl;
    std::cout << "Got output scores: " << scores_data[0] << std::endl;
}