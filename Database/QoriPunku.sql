-- phpMyAdmin SQL Dump
-- version 5.2.1
-- https://www.phpmyadmin.net/
--
-- Servidor: 127.0.0.1
-- Tiempo de generación: 19-04-2025 a las 06:40:43
-- Versión del servidor: 10.4.32-MariaDB
-- Versión de PHP: 8.2.12

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Base de datos: `qoripunku`
--

-- --------------------------------------------------------

--
-- Estructura de tabla para la tabla `admin_config`
--

CREATE TABLE `admin_config` (
  `id` int(11) NOT NULL,
  `config_key` varchar(50) NOT NULL,
  `config_value` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Volcado de datos para la tabla `admin_config`
--

INSERT INTO `admin_config` (`id`, `config_key`, `config_value`) VALUES
(1, 'admin_number', '900000000');

-- --------------------------------------------------------

--
-- Estructura de tabla para la tabla `admin_password`
--

CREATE TABLE `admin_password` (
  `id` int(11) NOT NULL,
  `password` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Volcado de datos para la tabla `admin_password`
--

INSERT INTO `admin_password` (`id`, `password`) VALUES
(1, '11111');

-- --------------------------------------------------------

--
-- Estructura de tabla para la tabla `allowed_uuids`
--

CREATE TABLE `allowed_uuids` (
  `id` int(11) NOT NULL,
  `uuid` varchar(20) NOT NULL,
  `status` enum('allowed','denied') NOT NULL DEFAULT 'denied',
  `created_at` timestamp NULL DEFAULT current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- --------------------------------------------------------

--
-- Estructura de tabla para la tabla `uploaded_images`
--

CREATE TABLE `uploaded_images` (
  `photo_id` varchar(255) NOT NULL,
  `msid` varchar(255) NOT NULL,
  `status` enum('pending','approved','denied') DEFAULT 'pending',
  `msid_user_response` varchar(255) DEFAULT NULL,
  `from_number` varchar(512) NOT NULL,
  `created_at` timestamp NULL DEFAULT current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Índices para tablas volcadas
--

--
-- Indices de la tabla `admin_config`
--
ALTER TABLE `admin_config`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `config_key` (`config_key`);

--
-- Indices de la tabla `admin_password`
--
ALTER TABLE `admin_password`
  ADD PRIMARY KEY (`id`);

--
-- Indices de la tabla `allowed_uuids`
--
ALTER TABLE `allowed_uuids`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `uuid` (`uuid`);

--
-- Indices de la tabla `uploaded_images`
--
ALTER TABLE `uploaded_images`
  ADD PRIMARY KEY (`photo_id`);

--
-- AUTO_INCREMENT de las tablas volcadas
--

--
-- AUTO_INCREMENT de la tabla `admin_config`
--
ALTER TABLE `admin_config`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=21;

--
-- AUTO_INCREMENT de la tabla `admin_password`
--
ALTER TABLE `admin_password`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;

--
-- AUTO_INCREMENT de la tabla `allowed_uuids`
--
ALTER TABLE `allowed_uuids`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=11;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
