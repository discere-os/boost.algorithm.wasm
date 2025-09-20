/**
 * Boost.Algorithm WASM TypeScript Wrapper
 * Copyright (c) 2002-2004 Pavol Droba, Boost contributors
 * Copyright (c) 2025 Superstruct Ltd, New Zealand
 * Licensed under Boost Software License, Version 1.0
 *
 * WASM-enhanced implementation of Boost.Algorithm with SIMD optimization
 * Provides full API compatibility with C++ Boost.Algorithm while adding
 * modern TypeScript interfaces and transparent performance acceleration.
 */

import {
  BoostAlgorithmOptions,
  TrimResult,
  SearchResult,
  ComparisonResult,
  BenchmarkResult,
  ClampOptions,
  MinMaxResult,
  HexOptions,
  HexEncodeResult,
  HexDecodeResult,
  AlgorithmStats,
  WasmModule,
  ModuleFactory,
  BoostAlgorithmError,
  SIMDUnsupportedError,
  ModuleInitializationError
} from './types.ts';

/**
 * Boost.Algorithm WASM-Enhanced Library
 *
 * This class provides a modern TypeScript interface to Boost.Algorithm
 * with transparent WASM SIMD acceleration. All operations fall back
 * gracefully to standard implementations when SIMD is unavailable.
 */
class BoostAlgorithm {
  private module: WasmModule | null = null;
  private initialized = false;
  private simdAvailable = false;
  private stats: AlgorithmStats;

  constructor(private options: BoostAlgorithmOptions = {}) {
    this.options = {
      simdOptimizations: true,
      maxMemoryMB: 256,
      debug: false,
      ...options
    };

    this.stats = {
      totalOperations: 0,
      simdOperations: 0,
      simdUsagePercent: 0,
      totalTimeMs: 0,
      avgSIMDSpeedup: 0
    };
  }

  /**
   * Initialize the WASM module and detect SIMD capabilities
   */
  async initialize(): Promise<void> {
    if (this.initialized) return;

    try {
      const wasmBinary = await this.loadWasmBinary();
      const moduleFactory = await this.loadModuleFactory();

      this.module = await moduleFactory({
        wasmBinary,
        locateFile: (path: string) => {
          if (path.endsWith('.wasm')) {
            return new URL('../../install/wasm/' + path, import.meta.url).href;
          }
          return path;
        }
      });

      // Check SIMD availability
      this.simdAvailable = this.options.simdOptimizations && this.module._boost_simd_available();

      this.initialized = true;

      if (this.options.debug) {
        console.log(`BoostAlgorithm initialized - SIMD: ${this.simdAvailable ? 'ENABLED' : 'DISABLED'}`);
      }

    } catch (error) {
      throw new ModuleInitializationError(error instanceof Error ? error.message : String(error));
    }
  }

  private async loadWasmBinary(): Promise<ArrayBuffer | undefined> {
    if (typeof globalThis.Deno !== 'undefined') {
      try {
        const wasmPath = new URL('../../install/wasm/boost-algorithm-main.wasm', import.meta.url).pathname;
        const wasmBuffer = await Deno.readFile(wasmPath);
        return wasmBuffer.buffer;
      } catch (error) {
        if (this.options.debug) {
          console.warn('Failed to load local WASM binary:', error);
        }
        return undefined;
      }
    }

    // Try CDN locations
    const cdnUrls = [
      'https://wasm.discere.cloud/boost.algorithm/latest/main/',
      'https://cdn.jsdelivr.net/npm/@discere-os/boost.algorithm.wasm/dist/'
    ];

    for (const url of cdnUrls) {
      try {
        const response = await fetch(`${url}boost-algorithm-main.wasm`);
        if (response.ok) {
          return await response.arrayBuffer();
        }
      } catch { continue; }
    }

    return undefined; // Fallback to embedded WASM
  }

  private async loadModuleFactory(): Promise<ModuleFactory> {
    if (typeof globalThis.Deno !== 'undefined') {
      const moduleFactory = (await import('../../install/wasm/boost-algorithm-main.js')).default;
      return moduleFactory;
    }

    // Try CDN locations
    const cdnUrls = [
      'https://wasm.discere.cloud/boost.algorithm/latest/main/',
      'https://cdn.jsdelivr.net/npm/@discere-os/boost.algorithm.wasm/dist/'
    ];

    for (const url of cdnUrls) {
      try {
        const moduleFactory = (await import(`${url}boost-algorithm-main.js`)).default;
        return moduleFactory;
      } catch { continue; }
    }

    throw new Error('Failed to load module factory from any source');
  }

  private ensureInitialized(): void {
    if (!this.initialized || !this.module) {
      throw new BoostAlgorithmError('Module not initialized. Call initialize() first.');
    }
  }

  private allocateString(str: string): number {
    const encoder = new TextEncoder();
    const bytes = encoder.encode(str);
    const ptr = this.module!._malloc(bytes.length + 1);
    this.module!.HEAPU8.set(bytes, ptr);
    this.module!.HEAPU8[ptr + bytes.length] = 0; // Null terminator
    return ptr;
  }

  private readString(ptr: number): string {
    return this.module!.UTF8ToString(ptr);
  }

  private trackOperation(simdUsed: boolean, timeMs: number = 0): void {
    this.stats.totalOperations++;
    this.stats.totalTimeMs += timeMs;

    if (simdUsed) {
      this.stats.simdOperations++;
    }

    this.stats.simdUsagePercent = (this.stats.simdOperations / this.stats.totalOperations) * 100;
  }

  // ============================================================================
  // STRING TRIMMING OPERATIONS
  // ============================================================================

  /**
   * Remove leading whitespace from string
   *
   * @param input - Input string to trim
   * @returns Trim result with performance metrics
   */
  trimLeft(input: string): TrimResult {
    this.ensureInitialized();
    const startTime = performance.now();

    if (this.simdAvailable && input.length > 16) {
      const ptr = this.allocateString(input);
      const lengthPtr = this.module!._malloc(4);
      this.module!.HEAP32[lengthPtr >> 2] = input.length;

      this.module!._boost_trim_left_simd(ptr, lengthPtr);

      const newLength = this.module!.HEAP32[lengthPtr >> 2];
      const result = this.readString(ptr);

      this.module!._free(ptr);
      this.module!._free(lengthPtr);

      const endTime = performance.now();
      this.trackOperation(true, endTime - startTime);

      return {
        result: result.substring(0, newLength),
        removedCount: input.length - newLength,
        simdUsed: true
      };
    } else {
      // Fallback to standard trimming
      const trimmed = input.replace(/^[\s\t\n\r]+/, '');
      const endTime = performance.now();
      this.trackOperation(false, endTime - startTime);

      return {
        result: trimmed,
        removedCount: input.length - trimmed.length,
        simdUsed: false
      };
    }
  }

  /**
   * Remove trailing whitespace from string
   */
  trimRight(input: string): TrimResult {
    this.ensureInitialized();
    const startTime = performance.now();

    if (this.simdAvailable && input.length > 16) {
      const ptr = this.allocateString(input);
      const lengthPtr = this.module!._malloc(4);
      this.module!.HEAP32[lengthPtr >> 2] = input.length;

      this.module!._boost_trim_right_simd(ptr, lengthPtr);

      const newLength = this.module!.HEAP32[lengthPtr >> 2];
      const result = this.readString(ptr);

      this.module!._free(ptr);
      this.module!._free(lengthPtr);

      const endTime = performance.now();
      this.trackOperation(true, endTime - startTime);

      return {
        result: result.substring(0, newLength),
        removedCount: input.length - newLength,
        simdUsed: true
      };
    } else {
      const trimmed = input.replace(/[\s\t\n\r]+$/, '');
      const endTime = performance.now();
      this.trackOperation(false, endTime - startTime);

      return {
        result: trimmed,
        removedCount: input.length - trimmed.length,
        simdUsed: false
      };
    }
  }

  /**
   * Remove leading and trailing whitespace from string
   */
  trim(input: string): TrimResult {
    this.ensureInitialized();
    const startTime = performance.now();

    if (this.simdAvailable && input.length > 16) {
      const ptr = this.allocateString(input);
      const lengthPtr = this.module!._malloc(4);
      this.module!.HEAP32[lengthPtr >> 2] = input.length;

      this.module!._boost_trim_simd(ptr, lengthPtr);

      const newLength = this.module!.HEAP32[lengthPtr >> 2];
      const result = this.readString(ptr);

      this.module!._free(ptr);
      this.module!._free(lengthPtr);

      const endTime = performance.now();
      this.trackOperation(true, endTime - startTime);

      return {
        result: result.substring(0, newLength),
        removedCount: input.length - newLength,
        simdUsed: true
      };
    } else {
      const trimmed = input.trim();
      const endTime = performance.now();
      this.trackOperation(false, endTime - startTime);

      return {
        result: trimmed,
        removedCount: input.length - trimmed.length,
        simdUsed: false
      };
    }
  }

  // ============================================================================
  // CASE CONVERSION OPERATIONS
  // ============================================================================

  /**
   * Convert string to uppercase
   */
  toUpper(input: string): { result: string; simdUsed: boolean } {
    this.ensureInitialized();
    const startTime = performance.now();

    if (this.simdAvailable && input.length > 16) {
      const ptr = this.allocateString(input);
      this.module!._boost_to_upper_simd(ptr, input.length);
      const result = this.readString(ptr);
      this.module!._free(ptr);

      const endTime = performance.now();
      this.trackOperation(true, endTime - startTime);

      return { result, simdUsed: true };
    } else {
      const result = input.toUpperCase();
      const endTime = performance.now();
      this.trackOperation(false, endTime - startTime);

      return { result, simdUsed: false };
    }
  }

  /**
   * Convert string to lowercase
   */
  toLower(input: string): { result: string; simdUsed: boolean } {
    this.ensureInitialized();
    const startTime = performance.now();

    if (this.simdAvailable && input.length > 16) {
      const ptr = this.allocateString(input);
      this.module!._boost_to_lower_simd(ptr, input.length);
      const result = this.readString(ptr);
      this.module!._free(ptr);

      const endTime = performance.now();
      this.trackOperation(true, endTime - startTime);

      return { result, simdUsed: true };
    } else {
      const result = input.toLowerCase();
      const endTime = performance.now();
      this.trackOperation(false, endTime - startTime);

      return { result, simdUsed: false };
    }
  }

  // ============================================================================
  // STRING SEARCH OPERATIONS
  // ============================================================================

  /**
   * Find first occurrence of substring
   */
  findFirst(haystack: string, needle: string): SearchResult {
    this.ensureInitialized();
    const startTime = performance.now();

    if (this.simdAvailable && haystack.length > 32 && needle.length > 0) {
      const haystackPtr = this.allocateString(haystack);
      const needlePtr = this.allocateString(needle);

      const resultPtr = this.module!._boost_find_first_simd(
        haystackPtr, haystack.length,
        needlePtr, needle.length
      );

      const position = resultPtr === 0 ? -1 : resultPtr - haystackPtr;

      this.module!._free(haystackPtr);
      this.module!._free(needlePtr);

      const endTime = performance.now();
      this.trackOperation(true, endTime - startTime);

      return {
        position,
        found: position !== -1,
        simdUsed: true
      };
    } else {
      const position = haystack.indexOf(needle);
      const endTime = performance.now();
      this.trackOperation(false, endTime - startTime);

      return {
        position,
        found: position !== -1,
        simdUsed: false
      };
    }
  }

  /**
   * Boyer-Moore search algorithm
   */
  boyerMooreSearch(haystack: string, needle: string): SearchResult {
    this.ensureInitialized();
    const startTime = performance.now();

    if (this.simdAvailable && needle.length > 1) {
      const haystackPtr = this.allocateString(haystack);
      const needlePtr = this.allocateString(needle);

      const resultPtr = this.module!._boost_boyer_moore_simd(
        haystackPtr, haystack.length,
        needlePtr, needle.length
      );

      const position = resultPtr === 0 ? -1 : resultPtr - haystackPtr;

      this.module!._free(haystackPtr);
      this.module!._free(needlePtr);

      const endTime = performance.now();
      this.trackOperation(true, endTime - startTime);

      return {
        position,
        found: position !== -1,
        simdUsed: true
      };
    } else {
      // Fallback to simple search
      const position = haystack.indexOf(needle);
      const endTime = performance.now();
      this.trackOperation(false, endTime - startTime);

      return {
        position,
        found: position !== -1,
        simdUsed: false
      };
    }
  }

  // ============================================================================
  // STRING COMPARISON OPERATIONS
  // ============================================================================

  /**
   * Case-sensitive string equality
   */
  equals(str1: string, str2: string): ComparisonResult {
    this.ensureInitialized();
    const startTime = performance.now();

    if (this.simdAvailable && str1.length > 16 && str2.length > 16) {
      const ptr1 = this.allocateString(str1);
      const ptr2 = this.allocateString(str2);

      const equal = this.module!._boost_equals_simd(
        ptr1, str1.length,
        ptr2, str2.length
      );

      this.module!._free(ptr1);
      this.module!._free(ptr2);

      const endTime = performance.now();
      this.trackOperation(true, endTime - startTime);

      return { equal, simdUsed: true };
    } else {
      const equal = str1 === str2;
      const endTime = performance.now();
      this.trackOperation(false, endTime - startTime);

      return { equal, simdUsed: false };
    }
  }

  /**
   * Case-insensitive string equality
   */
  iequals(str1: string, str2: string): ComparisonResult {
    this.ensureInitialized();
    const startTime = performance.now();

    if (this.simdAvailable && str1.length > 16 && str2.length > 16) {
      const ptr1 = this.allocateString(str1);
      const ptr2 = this.allocateString(str2);

      const equal = this.module!._boost_iequals_simd(
        ptr1, str1.length,
        ptr2, str2.length
      );

      this.module!._free(ptr1);
      this.module!._free(ptr2);

      const endTime = performance.now();
      this.trackOperation(true, endTime - startTime);

      return { equal, simdUsed: true };
    } else {
      const equal = str1.toLowerCase() === str2.toLowerCase();
      const endTime = performance.now();
      this.trackOperation(false, endTime - startTime);

      return { equal, simdUsed: false };
    }
  }

  // ============================================================================
  // STRING PREDICATE OPERATIONS
  // ============================================================================

  /**
   * Check if string starts with prefix
   */
  startsWith(str: string, prefix: string): ComparisonResult {
    this.ensureInitialized();
    const startTime = performance.now();

    if (this.simdAvailable && str.length > 16) {
      const strPtr = this.allocateString(str);
      const prefixPtr = this.allocateString(prefix);

      const equal = this.module!._boost_starts_with_simd(
        strPtr, str.length,
        prefixPtr, prefix.length
      );

      this.module!._free(strPtr);
      this.module!._free(prefixPtr);

      const endTime = performance.now();
      this.trackOperation(true, endTime - startTime);

      return { equal, simdUsed: true };
    } else {
      const equal = str.startsWith(prefix);
      const endTime = performance.now();
      this.trackOperation(false, endTime - startTime);

      return { equal, simdUsed: false };
    }
  }

  /**
   * Check if string ends with suffix
   */
  endsWith(str: string, suffix: string): ComparisonResult {
    this.ensureInitialized();
    const startTime = performance.now();

    if (this.simdAvailable && str.length > 16) {
      const strPtr = this.allocateString(str);
      const suffixPtr = this.allocateString(suffix);

      const equal = this.module!._boost_ends_with_simd(
        strPtr, str.length,
        suffixPtr, suffix.length
      );

      this.module!._free(strPtr);
      this.module!._free(suffixPtr);

      const endTime = performance.now();
      this.trackOperation(true, endTime - startTime);

      return { equal, simdUsed: true };
    } else {
      const equal = str.endsWith(suffix);
      const endTime = performance.now();
      this.trackOperation(false, endTime - startTime);

      return { equal, simdUsed: false };
    }
  }

  // ============================================================================
  // UTILITY OPERATIONS
  // ============================================================================

  /**
   * Check if string is palindrome
   */
  isPalindrome(str: string): ComparisonResult {
    this.ensureInitialized();
    const startTime = performance.now();

    if (this.simdAvailable && str.length > 32) {
      const strPtr = this.allocateString(str);
      const equal = this.module!._boost_is_palindrome_simd(strPtr, str.length);
      this.module!._free(strPtr);

      const endTime = performance.now();
      this.trackOperation(true, endTime - startTime);

      return { equal, simdUsed: true };
    } else {
      const reversed = str.split('').reverse().join('');
      const equal = str === reversed;
      const endTime = performance.now();
      this.trackOperation(false, endTime - startTime);

      return { equal, simdUsed: false };
    }
  }

  /**
   * Clamp array values to range [min, max]
   */
  clampArray(data: Float32Array, options: ClampOptions): { simdUsed: boolean } {
    this.ensureInitialized();
    const startTime = performance.now();

    const useSIMD = (options.useSIMD !== false) && this.simdAvailable && data.length > 64;

    if (useSIMD) {
      const ptr = this.module!._malloc(data.length * 4);
      this.module!.HEAPF32.set(data, ptr >> 2);

      this.module!._boost_clamp_array_simd(ptr, data.length, options.min, options.max);

      data.set(this.module!.HEAPF32.subarray(ptr >> 2, (ptr >> 2) + data.length));
      this.module!._free(ptr);

      const endTime = performance.now();
      this.trackOperation(true, endTime - startTime);

      return { simdUsed: true };
    } else {
      for (let i = 0; i < data.length; i++) {
        data[i] = Math.max(options.min, Math.min(options.max, data[i]));
      }

      const endTime = performance.now();
      this.trackOperation(false, endTime - startTime);

      return { simdUsed: false };
    }
  }

  /**
   * Find minimum and maximum values in array
   */
  minMaxElement(data: Float32Array): MinMaxResult {
    this.ensureInitialized();
    const startTime = performance.now();

    if (this.simdAvailable && data.length > 64) {
      const dataPtr = this.module!._malloc(data.length * 4);
      const minPtr = this.module!._malloc(4);
      const maxPtr = this.module!._malloc(4);

      this.module!.HEAPF32.set(data, dataPtr >> 2);

      this.module!._boost_minmax_element_simd(dataPtr, data.length, minPtr, maxPtr);

      const min = this.module!.HEAPF32[minPtr >> 2];
      const max = this.module!.HEAPF32[maxPtr >> 2];

      this.module!._free(dataPtr);
      this.module!._free(minPtr);
      this.module!._free(maxPtr);

      const endTime = performance.now();
      this.trackOperation(true, endTime - startTime);

      return { min, max, simdUsed: true };
    } else {
      let min = data[0];
      let max = data[0];

      for (let i = 1; i < data.length; i++) {
        if (data[i] < min) min = data[i];
        if (data[i] > max) max = data[i];
      }

      const endTime = performance.now();
      this.trackOperation(false, endTime - startTime);

      return { min, max, simdUsed: false };
    }
  }

  // ============================================================================
  // BENCHMARKING AND STATISTICS
  // ============================================================================

  /**
   * Run performance benchmarks
   */
  async runBenchmarks(): Promise<BenchmarkResult[]> {
    this.ensureInitialized();

    const results: BenchmarkResult[] = [];

    // Trim benchmark
    const trimTime = this.module!._boost_benchmark_trim_performance(1000, 10000);
    results.push({
      operation: 'trim',
      timeMs: trimTime * 1000,
      iterations: 1000,
      opsPerSecond: 1000 / trimTime,
      throughputMBps: (1000 * 10000) / (trimTime * 1000 * 1000),
      simdUsed: this.simdAvailable
    });

    // Search benchmark
    const searchTime = this.module!._boost_benchmark_search_performance(10000, 1000, 10);
    results.push({
      operation: 'search',
      timeMs: searchTime * 1000,
      iterations: 10000,
      opsPerSecond: 10000 / searchTime,
      throughputMBps: (10000 * 1000) / (searchTime * 1000 * 1000),
      simdUsed: this.simdAvailable
    });

    // Case conversion benchmark
    const caseTime = this.module!._boost_benchmark_case_conversion_performance(1000, 10000);
    results.push({
      operation: 'case_conversion',
      timeMs: caseTime * 1000,
      iterations: 1000,
      opsPerSecond: 1000 / caseTime,
      throughputMBps: (1000 * 10000) / (caseTime * 1000 * 1000),
      simdUsed: this.simdAvailable
    });

    return results;
  }

  /**
   * Get algorithm usage statistics
   */
  getStats(): AlgorithmStats {
    return { ...this.stats };
  }

  /**
   * Check if SIMD is available and enabled
   */
  isSIMDAvailable(): boolean {
    return this.simdAvailable;
  }

  /**
   * Check if module is initialized
   */
  isInitialized(): boolean {
    return this.initialized;
  }

  /**
   * Clean up resources
   */
  cleanup(): void {
    if (this.module) {
      // WASM module cleanup is handled by Emscripten
      this.module = null;
      this.initialized = false;
      this.simdAvailable = false;
    }
  }
}

// Export everything
export * from './types.ts';

// Default export
export default BoostAlgorithm;
export { BoostAlgorithm };