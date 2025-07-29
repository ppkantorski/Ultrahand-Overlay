/********************************************************************************
 * File: ultra.hpp
 * Author: ppkantorski
 * Description: 
 *   'ultra.hpp' serves as a central include header for the Ultrahand Overlay project,
 *   bringing together a comprehensive suite of utility functions essential for the
 *   development and operation of custom overlays on the Nintendo Switch. This header
 *   provides consolidated access to functions facilitating debugging, string processing,
 *   file management, JSON manipulation, and more, enhancing the modularity and 
 *   reusability of code within the project.
 *
 *   These utilities are designed to operate independently, providing robust tools to
 *   support complex overlay functionalities and interactions.
 *
 *   For the latest updates and contributions, visit the project's GitHub repository:
 *   GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay
 *
 *   Note: This notice is integral to the project's documentation and must not be 
 *   altered or removed.
 *
 *  Licensed under both GPLv2 and CC-BY-4.0
 *  Copyright (c) 2024 ppkantorski
 ********************************************************************************/
#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

#ifndef ULTRA_HPP
#define ULTRA_HPP

// Include all functional headers used in the libUltra library

#include "global_vars.hpp"
#include "debug_funcs.hpp"
#include "string_funcs.hpp"
#include "get_funcs.hpp"
#include "path_funcs.hpp"
#include "list_funcs.hpp"
#include "json_funcs.hpp"
#include "ini_funcs.hpp"
#include "hex_funcs.hpp"
#include "download_funcs.hpp"
#include "mod_funcs.hpp"
#include "tsl_utils.hpp"

#endif // ULTRA_HPP