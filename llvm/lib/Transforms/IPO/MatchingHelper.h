#ifndef MATCHINGHELPER_H
#define MATCHINGHELPER_H

#include <vector>
#include <iostream>
#include <tensorflow/c/c_api.h>

class MatchingHelper {
private:
    std::string model_path;
    std::string modelName;
    TF_Graph* graph = TF_NewGraph();
    TF_SessionOptions* session_options = TF_NewSessionOptions();
    TF_Status* status = TF_NewStatus();
    TF_Session* session;
public:
    bool load_model(std::string model_name) {
        const char* tags[] = {"serve"};
        int num_tags = 1;

        session = TF_LoadSessionFromSavedModel(session_options, nullptr, model_path.c_str(), tags, num_tags, graph, nullptr, status);
        if (TF_GetCode(status) != TF_OK) {
            llvm::dbgs() << "Error loading model: " << TF_Message(status) << "\n";
            return false;
        }
        return true;
    }

    // Constructor
    MatchingHelper(std::string TYP_Directory, std::string model_name = "MultiHeadAttention") {
        modelName = model_name;
        model_path = TYP_Directory + "/Model/log/" + model_name + "/";
        if (!load_model(model_name)) {
            llvm::dbgs() << "Error loading model" << "\n";
        }
    }

    std::vector<float> predict_value(const std::vector<float>& input1, const std::vector<float>& input2) {
        // Check if inputs have the correct dimensions
        if (input1.size() % 300 != 0 || input2.size() % 300 != 0 || input1.size() != input2.size()) {
            llvm::dbgs() << "Invalid input dimensions. Inputs must be multiples of 300 and have the same batch size." << "\n";
            return {};
        }

        // Calculate batch size
        int batch_size = input1.size() / 300;

        // Create input tensors
        TF_Status* status = TF_NewStatus();

        // Create tensor for input1
        int64_t dims1[2] = {batch_size, 300};
        TF_Tensor* input_tensor1 = TF_AllocateTensor(TF_FLOAT, dims1, 2, sizeof(float) * input1.size());
        std::memcpy(TF_TensorData(input_tensor1), input1.data(), sizeof(float) * input1.size());

        // Create tensor for input2
        int64_t dims2[2] = {batch_size, 300};
        TF_Tensor* input_tensor2 = TF_AllocateTensor(TF_FLOAT, dims2, 2, sizeof(float) * input2.size());
        std::memcpy(TF_TensorData(input_tensor2), input2.data(), sizeof(float) * input2.size());

        // Set up input tensors and operations
        TF_Output input_op1, input_op2;
        if (modelName == "MultiHeadAttention") {
            input_op1 = {TF_GraphOperationByName(graph, "serving_default_input_vec1"), 0};
            input_op2 = {TF_GraphOperationByName(graph, "serving_default_input_vec2"), 0};
        }
        else if (modelName == "DotProdSiameseModel") {
            input_op1 = {TF_GraphOperationByName(graph, "serving_default_input_layer_1"), 0};
            input_op2 = {TF_GraphOperationByName(graph, "serving_default_input_layer_2"), 0};
        }

        if (input_op1.oper == nullptr || input_op2.oper == nullptr) {
            llvm::dbgs() << "Error: Input operations not found in the model" << "\n";
            TF_DeleteTensor(input_tensor1);
            TF_DeleteTensor(input_tensor2);
            TF_DeleteStatus(status);
            return {};
        }

        // Set up output tensor
        TF_Output output_op = {TF_GraphOperationByName(graph, "StatefulPartitionedCall_1"), 0};
        if (output_op.oper == nullptr) {
            llvm::dbgs() << "Error: Output operation not found in the model" << "\n";
            TF_DeleteTensor(input_tensor1);
            TF_DeleteTensor(input_tensor2);
            TF_DeleteStatus(status);
            return {};
        }

        // Prepare for session run
        TF_Output inputs[2] = {input_op1, input_op2};
        TF_Tensor* input_values[2] = {input_tensor1, input_tensor2};
        TF_Output outputs[1] = {output_op};
        TF_Tensor* output_values[1] = {nullptr};

        // Run the session
        TF_SessionRun(
            session,
            nullptr,
            inputs, input_values, 2,
            outputs, output_values, 1,
            nullptr, 0,
            nullptr,
            status
        );

        // Check session run status
        if (TF_GetCode(status) != TF_OK) {
            llvm::dbgs() << "Error running session: " << TF_Message(status) << "\n";
            TF_DeleteTensor(input_tensor1);
            TF_DeleteTensor(input_tensor2);
            TF_DeleteStatus(status);
            return {};
        }

        // Extract results
        float* result_data = static_cast<float*>(TF_TensorData(output_values[0]));
        size_t result_size = TF_TensorElementCount(output_values[0]);
        std::vector<float> result(result_data, result_data + result_size);

        // Clean up
        TF_DeleteTensor(input_tensor1);
        TF_DeleteTensor(input_tensor2);
        TF_DeleteTensor(output_values[0]);
        TF_DeleteStatus(status);

        return result;
    }

    std::vector<float> predict_in_batches(const std::vector<float>& input1, const std::vector<float>& input2, int batch_size=32) {
        // Check if inputs have the correct dimensions
        if (input1.size() % 300 != 0 || input2.size() % 300 != 0 || input1.size() != input2.size()) {
            llvm::dbgs() << "Invalid input dimensions. Inputs must be multiples of 300 and have the same size." << "\n";
            return {};
        }

        // Encoding Size
        const int item_size = 300;
        const int total_items = input1.size() / item_size;
        std::vector<float> all_results;

        // Process in batches
        for (int batch_start = 0; batch_start < total_items; batch_start += batch_size) {
            // Calculate actual batch size
            int current_batch_size = std::min(batch_size, total_items - batch_start);

            // Extract batch data
            std::vector<float> batch1(current_batch_size * item_size);
            std::vector<float> batch2(current_batch_size * item_size);

            // Calculate start position and size for the batch
            int src_start = batch_start * item_size;
            int elements_to_copy = current_batch_size * item_size;

            // Copy batched data from each input
            std::copy(input1.begin() + src_start, input1.begin() + src_start + elements_to_copy,
                batch1.begin());
            std::copy(input2.begin() + src_start, input2.begin() + src_start + elements_to_copy,
                batch2.begin());

            // Run prediction on this batch
            std::vector<float> batch_result = predict_value(batch1, batch2);

            // Append results
            all_results.insert(all_results.end(), batch_result.begin(), batch_result.end());
        }

        return all_results;
    }
};

#endif // MATCHINGHELPER_H
