#pragma once
#include "skewed_vns.h"
#include "IO_handlerV2.h"


class Algorithms_Runner
{
public:
	static Result run_skewed_vns(const std::string& input_path, const std::string& output_path,
		int num_vehicles, const SkewedVNSConfig config);
};
