#include"IO_handler_utils.h"

std::string IO_handler_utils::create_table(Result& solution)
{
    std::vector<std::string> table_headers = { "Route", "Remaining capacity", "Route cost", "Number of clients" };

    int column_number = solution.routes.size() + 1;
    std::vector<int> column_widths;

    int column_width = 0;
    for (auto e : table_headers)
    {
        column_widths.push_back(e.size());
    }

    ////////////////////////////
    std::ostringstream oss;
    int total_width = 0;
    for (auto a : column_widths)
    {
        total_width += a; // +3 for padding and separator
    }


    // Naglowki tabeli
    for (int i = 0; i < table_headers.size(); i++)
    {
        oss << std::setw(column_widths[i]) << std::left << table_headers[i];
        if (i < table_headers.size() - 1) oss << " | ";
    }
    oss << "\n";
    draw_horizontal_lines(oss, total_width);
    oss << "\n";


    for (int s = 0; s < solution.routes.size(); s++)
    {
        oss << std::setw(column_widths[0]) << std::left << solution.routes[s].vehicle_id << " | ";
        oss << std::setw(column_widths[1]) << std::left << solution.routes[s].remaining_capacity << " | ";
        oss << std::setw(column_widths[2]) << std::left << solution.routes[s].route_cost << " | ";
        oss << std::setw(column_widths[3]) << std::left << solution.routes[s].customers.size() << " | ";
        oss << "\n";
        draw_horizontal_lines(oss, total_width);
        oss << "\n";
    }

    std::string result = oss.str();
    std::cout << result;
    return result;
}


void IO_handler_utils::draw_horizontal_lines(std::ostringstream& oss, int width)
{
    for (int i = 0; i < width + 10; i++)
    {
        oss << "-";
    }
}