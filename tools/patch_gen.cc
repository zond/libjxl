#include "lib/jxl/enc_patch_dictionary.h"
#include "lib/jxl/dec_patch_dictionary.h"
#include "lib/jxl/enc_bit_writer.h"
#include "lib/jxl/enc_aux_out.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include "no_memory_manager.h"

using namespace jxl;

// Encodes a PatchDictionary into a PaddedBytes (must be byte-aligned)
static PaddedBytes EncodeDict(PatchDictionary& dict) {
    BitWriter writer(jpegxl::tools::NoMemoryManager());
    if (!PatchDictionaryEncoder::Encode(
        dict, &writer, LayerType::Dictionary, /*aux_out=*/nullptr)) {
            throw std::exception();
        }
    writer.ZeroPadToByte();
    return std::move(writer).TakeBytes();
}

// Prints a byte array as a Rust static byte slice
static void PrintArray(const PaddedBytes& bytes, const std::string& name) {
    // Rust expects identifiers, so name should already be uppercase with underscores
    std::cout << "static " << name << ": &[u8] = &[";
    for (size_t i = 0; i < bytes.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(bytes[i]);
    }
    std::cout << std::dec << "];" << "\n";
}

// struct PatchReferencePosition {
//     size_t ref, x0, y0, xsize, ysize;
//   };
  
//   struct PatchPosition {
//     // Position of top-left corner of the patch in the image.
//     size_t x, y;
//     size_t ref_pos_idx;
//   };
  
int main() {
    // TEST CASE 2: Single patch
    PatchDictionary single_dict(nullptr);
    std::vector<PatchPosition> positions = {{10, 20, 0}};
    std::vector<PatchReferencePosition> ref_positions = {{0, 0, 0, 1, 1}};
    std::vector<PatchBlending> blendings = {{PatchBlendMode::kAdd, 0, false}};
    PatchDictionaryEncoder::SetPositions(
        &single_dict, positions, ref_positions, blendings, /*stride=*/1);
    single_dict.print();
    std::cerr << "\n";
    auto bytes = EncodeDict(single_dict);
    PrintArray(bytes, "SINGLE_PATCH_DICT");

    // TEST CASE 3: Multiple patches
    PatchDictionary multi_dict(nullptr);
    std::vector<PatchPosition> pos2 = {{0, 0, 0}};
    std::vector<PatchReferencePosition> ref2 = {{0, 0, 0, 2, 1}};
    std::vector<PatchBlending> blend2 = {{PatchBlendMode::kBlendAbove, 1, false}};

    pos2.push_back({5, 5, 1});
    ref2.push_back({0, 0, 0, 1, 2});
    blend2.push_back({PatchBlendMode::kMul, 0, true});
    blend2.push_back({PatchBlendMode::kMul, 0, true});
    blend2.push_back({PatchBlendMode::kMul, 0, true});
    blend2.push_back({PatchBlendMode::kMul, 0, true});
    blend2.push_back({PatchBlendMode::kMul, 0, true});

    PatchDictionaryEncoder::SetPositions(
        &multi_dict, pos2, ref2, blend2, /*stride=*/3);
    bytes = EncodeDict(multi_dict);
    multi_dict.print();
    std::cerr << "\n";
    PrintArray(bytes, "MULTI_PATCH_DICT");

    // TEST CASE 4: Large 3x2 patch without clamp
    PatchDictionary large_dict(nullptr);
    std::vector<PatchPosition> pos4 = {{2, 3, 0}};
    std::vector<PatchReferencePosition> ref4 = {{0, 0, 0, 300, 200}};
    std::vector<PatchBlending> blend4 = {{PatchBlendMode::kAlphaWeightedAddBelow, 0, false}, {PatchBlendMode::kMul, 0, false}};
    PatchDictionaryEncoder::SetPositions(
        &large_dict, pos4, ref4, blend4, /*stride=*/2);
    bytes = EncodeDict(large_dict);
    large_dict.print();
    std::cerr << "\n";
    PrintArray(bytes, "LARGE_PATCH_DICT");

    // TEST CASE 5: Single-pixel patch with clamp
    PatchDictionary clamp_dict(nullptr);
    std::vector<PatchPosition> pos5 = {{4, 4, 0}};
    std::vector<PatchReferencePosition> ref5 = {{0, 0, 0, 1, 1}};
    std::vector<PatchBlending> blend5 = {{PatchBlendMode::kMul, 0, true}};
    PatchDictionaryEncoder::SetPositions(
        &clamp_dict, pos5, ref5, blend5, /*stride=*/1);
    bytes = EncodeDict(clamp_dict);
    clamp_dict.print();
    std::cerr << "\n";
    PrintArray(bytes, "CLAMP_PATCH_DICT");

    // TEST CASE 6: Duplicate-patch references
    PatchDictionary dup_dict(nullptr);
    std::vector<PatchPosition> pos6 = {{0, 0, 0}, {5, 5, 0}};
    std::vector<PatchReferencePosition> ref6 = {{0, 0, 0, 1, 1}};
    std::vector<PatchBlending> blend6 = {{PatchBlendMode::kAdd, 0, false}, {PatchBlendMode::kAdd, 0, false}};
    PatchDictionaryEncoder::SetPositions(
        &dup_dict, pos6, ref6, blend6, /*stride=*/1);
    bytes = EncodeDict(dup_dict);
    dup_dict.print();
    std::cerr << "\n";
    PrintArray(bytes, "DUP_PATCH_DICT");

    return 0;
}