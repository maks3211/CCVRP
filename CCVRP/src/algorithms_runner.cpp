//#include "algorithms_runner.h"
 

//PLIK TEN JEST WYKLUCZONY Z KOMPLIKACJI 
//TRZEBA KLIKNA PRAWYM NA TEN PLIK - WLASCIWOSCI - WYKLUCZ Z KOMPLIKACJI - POZMIENIAC WE WSZYSTKICH KONFIGACH TZN
//DLA DEBUG, RELASE 32, 64 BIT


#include <pybind11/pybind11.h>
#include <pybind11/stl.h> 

#include "skewed_vns.h"
#include "IO_handlerV2.h"


namespace py = pybind11;


int run_alg(int param1, double param2, std::string param3) {
    return param1 + (int)param2;
}

Result run_skewed_vns(const std::string& input_path, const std::string& output_path,
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

// Moduł pybind11
PYBIND11_MODULE(moj_modul, m) {
    m.doc() = "Moj modul c++";


	py::class_<SkewedVNSConfig>(m, "SkewedVNSConfig")
		.def(py::init<>())
		.def_readwrite("block_size", &SkewedVNSConfig::block_size)
		.def_readwrite("delta_alfa", &SkewedVNSConfig::delta_alfa)
		.def_readwrite("f_alfa", &SkewedVNSConfig::f_alfa)
		.def_readwrite("SVNS_max_no_improve", &SkewedVNSConfig::SVNS_max_no_improve);

	py::class_<Node>(m, "Node")
		.def(py::init<>())
		.def_readwrite("id", &Node::id)
		.def_readwrite("x", &Node::x)
		.def_readwrite("y", &Node::y)
		.def_readwrite("demand", &Node::demand);

	py::class_<Route>(m, "Route")
		.def(py::init<int, int>())
		.def_readwrite("customers", &Route::customers)
		.def_readwrite("vehicle_id", &Route::vehicle_id)
		.def_readwrite("remaining_capacity", &Route::remaining_capacity)
		.def_readwrite("initial_capacity", &Route::initial_capacity)
		.def_readwrite("route_cost", &Route::route_cost);

	py::class_<Result>(m, "Result")
		.def(py::init<>())
		.def_readwrite("routes", &Result::routes)
		.def_readwrite("total_cost", &Result::total_cost)
		.def_readwrite("duration_seconds", &Result::duration_seconds);



    m.def("run_alg", &run_alg,
        "Opis funkcji run_alg",
        py::arg("param1"),
        py::arg("param2"),
        py::arg("param3"));


	m.def("run_skewed_vns", &run_skewed_vns,
		"Runs the Skewed VNS algorithm on a given CVRP instance.",
		py::arg("input_path"),
		py::arg("output_path"),
		py::arg("num_vehicles"),
		py::arg("config"));
}



