// SPDX-License-Identifier: GPL-2.1-or-later
// Copyright 2024 Soeren Grunewald <soeren.grunewald@gmx.net>
/*
 * What we actually do can be done in the shell, e.g:
 *
 * PATH_TO_POWER=/sys/class/drm/card1/device/hwmon/hwmon3
 * test -d $PATH_TO_POWER || exit 1
 *
 * min_power=`cat $PATH_TO_POWER/power1_cap_min`
 * max_power=`cat $PATH_TO_POWER/power1_cap_max`
 * def_power=`cat $PATH_TO_POWER/power1_cap_default`
 *
 * echo $min_power | tee $PATH_TO_POWER/power1_cap
 */

#include <cerrno>
#include <cstdint>
#include <cstring>

#include <array>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <cxxopts.hpp>

namespace fs = std::filesystem;

namespace {

	constexpr inline bool starts_with(std::string_view str, std::string_view prefix) {
		auto const hsl = str.length();
		auto const hl = prefix.length();
		return hsl >= hl and std::string_view{ str.data(), hl }.compare(prefix) == 0;
	}

	std::optional<std::string> read_string_from(fs::path const& p) {
		std::ifstream f{p};
		if (not f.is_open())
			return {};
		std::string s;
		std::getline(f, s);
		return s;
	}

	std::optional<std::uint64_t> read_dec_uint64_value_from(fs::path const& p) {
		auto v = read_string_from(p);
		if (v.has_value()) try {
			return std::stoul(v.value());
		} catch (std::exception const& e) {
			std::cerr << "Unable to convert " << v.value() << " to unsigned value: " << e.what() << std::endl;
		}
		return {};
	}

	inline int write_dec_uint64_value_to(fs::path const& p, std::uint64_t v) {
		std::ofstream f{p};
		if (not f.is_open())
			return -EPERM;
		std::cout << "Trying to write " << (v / 1000) << " to " << p << "...\n";
		f << v;
		return 0;
	}

	inline int write_dec_uint64_value_to(fs::path const& p, std::optional<std::uint64_t> const& v) {
		if (not v.has_value())
			return -ENODATA;
		return write_dec_uint64_value_to(p, v.value());
	}

	// Try to find the first card entry
	std::string find_card_base_path() {
		fs::path const base_path{ "/sys/class/drm" };
		for (auto const& dir_entry : fs::directory_iterator{ base_path }) {
			if (not dir_entry.is_directory())
				continue;
			auto const p = dir_entry.path();
			if (not starts_with(p.filename().string(), "card"))
				continue;
			return p.string();
		}
		return "";
	}

	// Try to figure the hwmon entry
	std::string find_hwmon_base_path(fs::path const& p) {
		auto const base_path = p / "device/hwmon";
		for (auto const& dir_entry : fs::directory_iterator{ base_path }) {
			if (not dir_entry.is_directory())
				continue;
			return dir_entry.path().string();
		}
		return "";
	}

	enum Action {
		RestoreDefault = 0,
		SetToMin,
		SetToMax,
	};

	inline std::string_view to_string(Action a) {
		switch (a) {
		case Action::SetToMin: return "minimal";
		case Action::SetToMax: return "maximal";
		case Action::RestoreDefault: return "default";
		}
		return "";
	}
}

int main(int argc, char* argv[])
{
	cxxopts::Options options("powercap", "Set power-limits on AMD GPUs");
	options.add_options()
		("v,verbose", "Enable extra messages", cxxopts::value<bool>()->default_value("false"))
		("min", "Set power limits to minimum (default)")
		("max", "Set power limits to maximum")
		("default", "Restore driver default value")
		("h,help", "Print usage")
		;

	auto result = options.parse(argc, argv);
	if (result.count("help")) {
		std::cout << options.help() << std::endl;
		return 0;
	}

	Action what_to_do = Action::SetToMin;
	if (result.count("max"))
		what_to_do = Action::SetToMax;
	if (result.count("default"))
		what_to_do = Action::RestoreDefault;

	auto const verbose = result["verbose"].as<bool>();
	if (verbose)
		std::cout << "Setting power-target to " << to_string(what_to_do) << "...\n";

	auto const card = find_card_base_path();
	if (card.empty()) {
		std::cerr << "Unable to find gpu\n";
		return 1;
	}

    auto const hwmon = find_hwmon_base_path(card);
	if (hwmon.empty()) {
		std::cerr << "Unable to find hwmon entries for " << card << "\n";
		return 1;
	}

	static constexpr std::array<std::string_view, 3> pwr_source = {
		"/power1_cap_default",
		"/power1_cap_min",
		"/power1_cap_max"
	};

	auto pwrtarget = read_dec_uint64_value_from(hwmon + std::string{ pwr_source[what_to_do] });
	auto err = write_dec_uint64_value_to(hwmon + "/power1_cap", pwrtarget);
	if (err < 0)
		std::cerr << "Could not write " << std::strerror(-err) << std::endl;

	return 0;
}
