#ifndef REPRESENT_H
#define REPRESENT_H

#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <pplx/pplx.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>

using namespace web;
using namespace web::http;
using namespace web::http::client;

class Represent {
private:
    std::string api_base = "https://represent.opennorth.ca";
    http_client_config client_config;
    
    http_request create_request(const std::string& endpoint, const std::string& method) {
        http_request request;
        
        if (method == "GET") {
            request.set_method(methods::GET);
        } else if (method == "POST") {
            request.set_method(methods::POST);
        } else if (method == "PUT") {
            request.set_method(methods::PUT);
        } else if (method == "DEL") {
            request.set_method(methods::DEL);
        }
        
        request.set_request_uri(utility::conversions::to_string_t(endpoint));
        
        // Set headers
        request.headers().add(U("Accept"), U("application/json"));
        request.headers().add(U("Content-Type"), U("application/json"));
        request.headers().add(U("User-Agent"), U("Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/115.0"));
        
        return request;
    }
    
    std::string build_query_params(const std::map<std::string, std::string>& params) {
        if (params.empty()) return "";
        
        std::string query = "?";
        bool first = true;
        for (const auto& param : params) {
            if (!param.second.empty()) {
                if (!first) query += "&";
                auto encoded_value = web::uri::encode_data_string(utility::conversions::to_string_t(param.second));
                query += param.first + "=" + utility::conversions::to_utf8string(encoded_value);
                first = false;
            }
        }
        return query;
    }
    
    pplx::task<json::value> make_api_call(const std::string& endpoint, const std::string& method) {
        http_client client(utility::conversions::to_string_t(api_base), client_config);
        auto request = create_request(endpoint, method);

        return client.request(request)
            .then([](http_response response) {
                if (response.status_code() == status_codes::OK) {
                    return response.extract_json();
                } else {
                    json::value error_obj;
                    error_obj[U("error")] = json::value::string(
                        U("HTTP Error: ") + utility::conversions::to_string_t(std::to_string(response.status_code())));
                    error_obj[U("success")] = json::value::boolean(false);
                    return pplx::task_from_result(error_obj);
                }
            })
            .then([](pplx::task<json::value> previousTask) {
                try {
                    return previousTask.get();
                } catch (const std::exception& e) {
                    json::value error_obj;
                    error_obj[U("error")] = json::value::string(
                        U("Exception: ") + utility::conversions::to_string_t(e.what()));
                    error_obj[U("success")] = json::value::boolean(false);
                    return error_obj;
                }
            });
    }

public:
    Represent() {
        client_config.set_validate_certificates(false);
    }

    pplx::task<json::value> get_postcode_info(const std::string& postcode) {
        return make_api_call("/postcodes/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(postcode))) + "/", "GET");
    }

    pplx::task<json::value> get_postcode_info_with_sets(const std::string& postcode, const std::string& sets) {
        std::map<std::string, std::string> params;
        params["sets"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(sets)));
        return make_api_call("/postcodes/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(postcode))) + "/" + build_query_params(params), "GET");
    }

    pplx::task<json::value> get_boundary_sets() {
        return make_api_call("/boundary-sets/", "GET");
    }

    pplx::task<json::value> get_boundary_set(const std::string& boundary_set) {
        return make_api_call("/boundary-sets/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary_set))) + "/", "GET");
    }

    pplx::task<json::value> get_boundary_sets_by_domain(const std::string& domain) {
        std::map<std::string, std::string> params;
        params["domain"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(domain)));
        return make_api_call("/boundary-sets/" + build_query_params(params), "GET");
    }

    pplx::task<json::value> get_boundaries() {
        return make_api_call("/boundaries/", "GET");
    }

    pplx::task<json::value> get_boundaries_by_set(const std::string& boundary_set) {
        return make_api_call("/boundaries/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary_set))) + "/", "GET");
    }

    pplx::task<json::value> get_boundaries_by_multiple_sets(const std::string& sets) {
        std::map<std::string, std::string> params;
        params["sets"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(sets)));
        return make_api_call("/boundaries/" + build_query_params(params), "GET");
    }

    pplx::task<json::value> get_boundary(const std::string& boundary_set, const std::string& boundary) {
        return make_api_call("/boundaries/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary_set))) + "/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary))) + "/", "GET");
    }

    pplx::task<json::value> search_boundaries(const std::string& name = "", const std::string& external_id = "") {
        std::map<std::string, std::string> params;
        if (!name.empty()) params["name"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(name)));
        if (!external_id.empty()) params["external_id"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(external_id)));
        return make_api_call("/boundaries/" + build_query_params(params), "GET");
    }

    pplx::task<json::value> search_boundaries_in_set(const std::string& boundary_set, const std::string& name = "", const std::string& external_id = "") {
        std::map<std::string, std::string> params;
        if (!name.empty()) params["name"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(name)));
        if (!external_id.empty()) params["external_id"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(external_id)));
        return make_api_call("/boundaries/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary_set))) + "/" + build_query_params(params), "GET");
    }

    pplx::task<json::value> get_boundaries_by_point(double lat, double lng) {
        std::map<std::string, std::string> params;
        params["contains"] = std::to_string(lat) + "," + std::to_string(lng);
        return make_api_call("/boundaries/" + build_query_params(params), "GET");
    }

    pplx::task<json::value> get_boundaries_by_point_in_set(const std::string& boundary_set, double lat, double lng) {
        std::map<std::string, std::string> params;
        params["contains"] = std::to_string(lat) + "," + std::to_string(lng);
        return make_api_call("/boundaries/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary_set))) + "/" + build_query_params(params), "GET");
    }

    pplx::task<json::value> get_touching_boundaries(const std::string& boundary_id) {
        std::map<std::string, std::string> params;
        params["touches"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary_id)));
        return make_api_call("/boundaries/" + build_query_params(params), "GET");
    }

    pplx::task<json::value> get_intersecting_boundaries(const std::string& boundary_id) {
        std::map<std::string, std::string> params;
        params["intersects"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary_id)));
        return make_api_call("/boundaries/" + build_query_params(params), "GET");
    }

    pplx::task<json::value> get_simple_shapes(const std::string& boundary_set, const std::string& format = "") {
        std::map<std::string, std::string> params;
        if (!format.empty()) params["format"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(format)));
        return make_api_call("/boundaries/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary_set))) + "/simple_shape" + build_query_params(params), "GET");
    }

    pplx::task<json::value> get_original_shapes(const std::string& boundary_set, const std::string& format = "") {
        std::map<std::string, std::string> params;
        if (!format.empty()) params["format"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(format)));
        return make_api_call("/boundaries/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary_set))) + "/shape" + build_query_params(params), "GET");
    }

    pplx::task<json::value> get_centroids(const std::string& boundary_set) {
        return make_api_call("/boundaries/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary_set))) + "/centroid", "GET");
    }

    pplx::task<json::value> get_boundary_simple_shape(const std::string& boundary_set, const std::string& boundary, const std::string& format = "") {
        std::map<std::string, std::string> params;
        if (!format.empty()) params["format"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(format)));
        return make_api_call("/boundaries/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary_set))) + "/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary))) + "/simple_shape" + build_query_params(params), "GET");
    }

    pplx::task<json::value> get_boundary_original_shape(const std::string& boundary_set, const std::string& boundary, const std::string& format = "") {
        std::map<std::string, std::string> params;
        if (!format.empty()) params["format"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(format)));
        return make_api_call("/boundaries/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary_set))) + "/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary))) + "/shape" + build_query_params(params), "GET");
    }

    pplx::task<json::value> get_boundary_centroid(const std::string& boundary_set, const std::string& boundary) {
        return make_api_call("/boundaries/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary_set))) + "/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary))) + "/centroid", "GET");
    }

    pplx::task<json::value> get_representative_sets() {
        return make_api_call("/representative-sets/", "GET");
    }

    pplx::task<json::value> get_representative_set(const std::string& representative_set) {
        return make_api_call("/representative-sets/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(representative_set))) + "/", "GET");
    }


    pplx::task<json::value> get_representatives() {
        return make_api_call("/representatives/", "GET");
    }

    pplx::task<json::value> get_representatives_by_set(const std::string& representative_set) {
        return make_api_call("/representatives/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(representative_set))) + "/", "GET");
    }

    pplx::task<json::value> get_representatives_by_point(double lat, double lng) {
        std::map<std::string, std::string> params;
        params["point"] = std::to_string(lat) + "," + std::to_string(lng);
        return make_api_call("/representatives/" + build_query_params(params), "GET");
    }

    pplx::task<json::value> get_representatives_by_point_in_set(const std::string& representative_set, double lat, double lng) {
        std::map<std::string, std::string> params;
        params["point"] = std::to_string(lat) + "," + std::to_string(lng);
        return make_api_call("/representatives/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(representative_set))) + "/" + build_query_params(params), "GET");
    }

    pplx::task<json::value> get_representatives_for_boundary(const std::string& boundary_set, const std::string& boundary) {
        return make_api_call("/boundaries/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary_set))) + "/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(boundary))) + "/representatives/", "GET");
    }

    pplx::task<json::value> get_representatives_for_multiple_districts(const std::string& districts) {
        std::map<std::string, std::string> params;
        params["districts"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(districts)));
        return make_api_call("/representatives/" + build_query_params(params), "GET");
    }

    pplx::task<json::value> search_representatives(
        const std::string& name = "",
        const std::string& first_name = "",
        const std::string& last_name = "",
        const std::string& gender = "",
        const std::string& district_name = "",
        const std::string& elected_office = "",
        const std::string& party_name = "") {
        
        std::map<std::string, std::string> params;
        if (!name.empty()) params["name"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(name)));
        if (!first_name.empty()) params["first_name"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(first_name)));
        if (!last_name.empty()) params["last_name"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(last_name)));
        if (!gender.empty()) params["gender"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(gender)));
        if (!district_name.empty()) params["district_name"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(district_name)));
        if (!elected_office.empty()) params["elected_office"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(elected_office)));
        if (!party_name.empty()) params["party_name"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(party_name)));
        
        return make_api_call("/representatives/" + build_query_params(params), "GET");
    }

    pplx::task<json::value> search_representatives_in_set(
        const std::string& representative_set,
        const std::string& name = "",
        const std::string& first_name = "",
        const std::string& last_name = "",
        const std::string& gender = "",
        const std::string& district_name = "",
        const std::string& elected_office = "",
        const std::string& party_name = "") {
        
        std::map<std::string, std::string> params;
        if (!name.empty()) params["name"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(name)));
        if (!first_name.empty()) params["first_name"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(first_name)));
        if (!last_name.empty()) params["last_name"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(last_name)));
        if (!gender.empty()) params["gender"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(gender)));
        if (!district_name.empty()) params["district_name"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(district_name)));
        if (!elected_office.empty()) params["elected_office"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(elected_office)));
        if (!party_name.empty()) params["party_name"] = utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(party_name)));
        
        return make_api_call("/representatives/" + utility::conversions::to_utf8string(web::uri::encode_data_string(utility::conversions::to_string_t(representative_set))) + "/" + build_query_params(params), "GET");
    }

    pplx::task<json::value> get_elections() {
        return make_api_call("/elections/", "GET");
    }

    pplx::task<json::value> get_candidates() {
        return make_api_call("/candidates/", "GET");
    }
};

#endif
