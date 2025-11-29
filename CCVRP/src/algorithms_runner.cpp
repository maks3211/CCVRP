#include "algorithms_runner.h"

Result Algorithms_Runner::run_skewed_vns(const std::string& input_path, const std::string& output_path,
	int num_vehicles, const SkewedVNSConfig config)
{
	IO_handlerV2::IO_handler io;
	io.set_input_path(input_path);
	io.set_result_path(output_path);

	CVRPInstance instance = io.get_instance();

	Skewed_VNS alg(instance, num_vehicles, io);
	alg.config = config;
	alg.run();
	Result result = alg.get_result();
	io.save_solution(result);
	return result;
}
