/*
 * Boost.Algorithm WASM TypeScript Type Definitions
 * Copyright (c) 2002-2004 Pavol Droba, Boost contributors
 * Copyright (c) 2025 Superstruct Ltd, New Zealand
 * Licensed under Boost Software License, Version 1.0
 */

/**
 * Configuration options for Boost.Algorithm WASM module
 */
export interface BoostAlgorithmOptions {
  /** Enable SIMD optimizations (auto-detected by default) */
  simdOptimizations?: boolean;
  /** Maximum memory usage in MB */
  maxMemoryMB?: number;
  /** Enable debug logging */
  debug?: boolean;
}

/**
 * String trimming result
 */
export interface TrimResult {
  /** Trimmed string */
  result: string;
  /** Number of characters removed */
  removedCount: number;
  /** Whether SIMD optimization was used */
  simdUsed: boolean;
}

/**
 * String search result
 */
export interface SearchResult {
  /** Position of found substring (-1 if not found) */
  position: number;
  /** Whether the substring was found */
  found: boolean;
  /** Whether SIMD optimization was used */
  simdUsed: boolean;
}

/**
 * String comparison result
 */
export interface ComparisonResult {
  /** Whether strings are equal */
  equal: boolean;
  /** Whether SIMD optimization was used */
  simdUsed: boolean;
}

/**
 * Performance benchmark result
 */
export interface BenchmarkResult {
  /** Operation name */
  operation: string;
  /** Time taken in milliseconds */
  timeMs: number;
  /** Iterations performed */
  iterations: number;
  /** Operations per second */
  opsPerSecond: number;
  /** Throughput in MB/s (if applicable) */
  throughputMBps?: number;
  /** Whether SIMD was used */
  simdUsed: boolean;
}

/**
 * Array clamp operation options
 */
export interface ClampOptions {
  /** Minimum value */
  min: number;
  /** Maximum value */
  max: number;
  /** Whether to use SIMD optimization */
  useSIMD?: boolean;
}

/**
 * MinMax result for arrays
 */
export interface MinMaxResult {
  /** Minimum value found */
  min: number;
  /** Maximum value found */
  max: number;
  /** Whether SIMD optimization was used */
  simdUsed: boolean;
}

/**
 * Hex encoding/decoding options
 */
export interface HexOptions {
  /** Use uppercase letters for encoding */
  uppercase?: boolean;
  /** Whether to use SIMD optimization */
  useSIMD?: boolean;
}

/**
 * Hex encoding result
 */
export interface HexEncodeResult {
  /** Encoded hex string */
  hex: string;
  /** Whether SIMD optimization was used */
  simdUsed: boolean;
}

/**
 * Hex decoding result
 */
export interface HexDecodeResult {
  /** Decoded bytes */
  bytes: Uint8Array;
  /** Whether decoding was successful */
  success: boolean;
  /** Whether SIMD optimization was used */
  simdUsed: boolean;
}

/**
 * String algorithm statistics
 */
export interface AlgorithmStats {
  /** Total operations performed */
  totalOperations: number;
  /** Operations that used SIMD */
  simdOperations: number;
  /** SIMD usage percentage */
  simdUsagePercent: number;
  /** Total time spent in algorithms (ms) */
  totalTimeMs: number;
  /** Average performance improvement with SIMD */
  avgSIMDSpeedup: number;
}

/**
 * WASM module interface (internal)
 */
export interface WasmModule {
  // Memory management
  _malloc(size: number): number;
  _free(ptr: number): void;

  // String conversion utilities
  UTF8ToString(ptr: number): string;
  HEAPU8: Uint8Array;
  HEAP32: Int32Array;
  HEAPF32: Float32Array;

  // Core SIMD functions (C API)
  _boost_simd_available(): boolean;
  _boost_trim_left_simd(str: number, len: number): void;
  _boost_trim_right_simd(str: number, len: number): void;
  _boost_trim_simd(str: number, len: number): void;
  _boost_to_upper_simd(str: number, len: number): void;
  _boost_to_lower_simd(str: number, len: number): void;
  _boost_find_first_simd(haystack: number, haystack_len: number, needle: number, needle_len: number): number;
  _boost_equals_simd(s1: number, len1: number, s2: number, len2: number): boolean;
  _boost_iequals_simd(s1: number, len1: number, s2: number, len2: number): boolean;
  _boost_starts_with_simd(str: number, str_len: number, prefix: number, prefix_len: number): boolean;
  _boost_ends_with_simd(str: number, str_len: number, suffix: number, suffix_len: number): boolean;
  _boost_boyer_moore_simd(haystack: number, haystack_len: number, needle: number, needle_len: number): number;
  _boost_kmp_simd(haystack: number, haystack_len: number, needle: number, needle_len: number): number;
  _boost_clamp_array_simd(data: number, len: number, min_val: number, max_val: number): void;
  _boost_clamp_array_int_simd(data: number, len: number, min_val: number, max_val: number): void;
  _boost_minmax_element_simd(data: number, len: number, min_val: number, max_val: number): void;
  _boost_minmax_element_int_simd(data: number, len: number, min_val: number, max_val: number): void;
  _boost_hex_decode_simd(hex_str: number, hex_len: number, output: number): boolean;
  _boost_hex_encode_simd(data: number, data_len: number, output: number): void;
  _boost_is_palindrome_simd(str: number, len: number): boolean;

  // Benchmarking functions
  _boost_benchmark_trim_performance(iterations: number, string_len: number): number;
  _boost_benchmark_search_performance(iterations: number, haystack_len: number, needle_len: number): number;
  _boost_benchmark_case_conversion_performance(iterations: number, string_len: number): number;

  // Emscripten utilities
  cwrap(name: string, returnType: string, argTypes: string[]): Function;
  ccall(name: string, returnType: string, argTypes: string[], args: any[]): any;
}

/**
 * Module factory function type
 */
export type ModuleFactory = (options?: any) => Promise<WasmModule>;

/**
 * Error types
 */
export class BoostAlgorithmError extends Error {
  constructor(message: string, public code?: string) {
    super(message);
    this.name = 'BoostAlgorithmError';
  }
}

export class SIMDUnsupportedError extends BoostAlgorithmError {
  constructor() {
    super('WASM SIMD not supported in this environment', 'SIMD_UNSUPPORTED');
  }
}

export class ModuleInitializationError extends BoostAlgorithmError {
  constructor(cause: string) {
    super(`Failed to initialize WASM module: ${cause}`, 'MODULE_INIT_FAILED');
  }
}

/**
 * Re-export common types for convenience
 */
export type {
  BoostAlgorithmOptions as Options,
  TrimResult,
  SearchResult,
  ComparisonResult,
  BenchmarkResult,
  ClampOptions,
  MinMaxResult,
  HexOptions,
  HexEncodeResult,
  HexDecodeResult,
  AlgorithmStats
};