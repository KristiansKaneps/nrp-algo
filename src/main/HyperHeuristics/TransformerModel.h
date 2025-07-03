#ifndef TRANSFORMERMODEL_H
#define TRANSFORMERMODEL_H

#include <cmath>

#include <torch/torch.h>

namespace HyperHeuristics {
    struct TransformerModelImpl : torch::nn::Module {
        double d_model_sqrt;

        torch::nn::Linear input_proj{nullptr};
        torch::nn::TransformerEncoderLayer encoder_layer{nullptr};
        torch::nn::TransformerEncoder encoder{nullptr};
        torch::nn::Linear output_head{nullptr};

        TransformerModelImpl(int input_dim, int d_model, int nhead, int num_layers, int output_dim) noexcept : d_model_sqrt(std::sqrt(static_cast<double>(d_model))) {
            assert(input_dim > 0 && d_model > 0 && nhead > 0 && num_layers > 0 && output_dim > 0);
            assert(d_model % nhead == 0 && "d_model must be divisible by nhead");

            input_proj = register_module("input_proj", torch::nn::Linear(input_dim, d_model));

            encoder_layer = register_module("encoder_layer", torch::nn::TransformerEncoderLayer(
                torch::nn::TransformerEncoderLayerOptions(d_model, nhead)
                    .activation(torch::kGELU)
                    .dropout(0.1)
            ));

            encoder = register_module("encoder", torch::nn::TransformerEncoder(
                encoder_layer, num_layers
            ));

            output_head = register_module("output_head", torch::nn::Linear(d_model, output_dim));
        }

        torch::Tensor forward(torch::Tensor x) noexcept {
            x = input_proj(x) * d_model_sqrt;
            x = x.transpose(0, 1); // [SeqLen, Batch, d_model]
            x = encoder(x);
            x = x.transpose(0, 1); // [Batch, SeqLen, d_model]
            return output_head(x); // [Batch, SeqLen, output_dim]
        }
    };

    TORCH_MODULE(TransformerModel);
}

#endif //TRANSFORMERMODEL_H
