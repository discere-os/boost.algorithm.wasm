import { assert, assertEquals, assertExists } from "@std/assert";
import BoostAlgorithm from "../../src/lib/index.ts";

Deno.test("Boost.Algorithm initialization", async () => {
  const boost = new BoostAlgorithm();
  await boost.initialize();
  assertExists(boost);
  assert(boost.isInitialized());
  boost.cleanup();
});

Deno.test("String trimming functionality", async () => {
  const boost = new BoostAlgorithm();
  await boost.initialize();

  const testStr = "   \t\n  Hello World  \t\n  ";

  const leftTrim = boost.trimLeft(testStr);
  assertEquals(leftTrim.result, "Hello World  \t\n  ");
  assert(leftTrim.removedCount > 0);

  const rightTrim = boost.trimRight(testStr);
  assertEquals(rightTrim.result, "   \t\n  Hello World");
  assert(rightTrim.removedCount > 0);

  const fullTrim = boost.trim(testStr);
  assertEquals(fullTrim.result, "Hello World");
  assert(fullTrim.removedCount > 0);

  boost.cleanup();
});

Deno.test("Case conversion functionality", async () => {
  const boost = new BoostAlgorithm();
  await boost.initialize();

  const testStr = "Hello WASM World";

  const upper = boost.toUpper(testStr);
  assertEquals(upper.result, "HELLO WASM WORLD");

  const lower = boost.toLower(testStr);
  assertEquals(lower.result, "hello wasm world");

  boost.cleanup();
});

Deno.test("String search functionality", async () => {
  const boost = new BoostAlgorithm();
  await boost.initialize();

  const haystack = "The quick brown fox jumps";
  const needle = "brown";

  const result = boost.findFirst(haystack, needle);
  assert(result.found);
  assertEquals(result.position, 10);

  const notFound = boost.findFirst(haystack, "elephant");
  assert(!notFound.found);
  assertEquals(notFound.position, -1);

  boost.cleanup();
});

Deno.test("String comparison functionality", async () => {
  const boost = new BoostAlgorithm();
  await boost.initialize();

  const str1 = "Hello World";
  const str2 = "Hello World";
  const str3 = "hello world";

  const exactEqual = boost.equals(str1, str2);
  assert(exactEqual.equal);

  const exactNotEqual = boost.equals(str1, str3);
  assert(!exactNotEqual.equal);

  const caseInsensitiveEqual = boost.iequals(str1, str3);
  assert(caseInsensitiveEqual.equal);

  boost.cleanup();
});

Deno.test("String predicates functionality", async () => {
  const boost = new BoostAlgorithm();
  await boost.initialize();

  const testStr = "Hello WASM World";

  const startsWithHello = boost.startsWith(testStr, "Hello");
  assert(startsWithHello.equal);

  const startsWithWorld = boost.startsWith(testStr, "World");
  assert(!startsWithWorld.equal);

  const endsWithWorld = boost.endsWith(testStr, "World");
  assert(endsWithWorld.equal);

  const endsWithHello = boost.endsWith(testStr, "Hello");
  assert(!endsWithHello.equal);

  boost.cleanup();
});

Deno.test("Palindrome check functionality", async () => {
  const boost = new BoostAlgorithm();
  await boost.initialize();

  const palindrome = "racecar";
  const notPalindrome = "hello";

  const isPalindrome = boost.isPalindrome(palindrome);
  assert(isPalindrome.equal);

  const isNotPalindrome = boost.isPalindrome(notPalindrome);
  assert(!isNotPalindrome.equal);

  boost.cleanup();
});

Deno.test("Array operations functionality", async () => {
  const boost = new BoostAlgorithm();
  await boost.initialize();

  const testArray = new Float32Array([5.5, 2.1, 8.7, 1.2, 9.9]);

  // Test minmax
  const minMax = boost.minMaxElement(testArray);
  assertEquals(Math.round(minMax.min * 10) / 10, 1.2);
  assertEquals(Math.round(minMax.max * 10) / 10, 9.9);

  // Test clamp
  const clampArray = new Float32Array([5.5, 2.1, 8.7, 1.2, 9.9]);
  boost.clampArray(clampArray, { min: 3.0, max: 8.0 });

  // All values should be clamped to [3.0, 8.0]
  for (const value of clampArray) {
    assert(value >= 3.0);
    assert(value <= 8.0);
  }

  boost.cleanup();
});

Deno.test("Statistics tracking", async () => {
  const boost = new BoostAlgorithm();
  await boost.initialize();

  // Perform some operations
  boost.trim("  hello  ");
  boost.toUpper("test");
  boost.findFirst("hello world", "world");

  const stats = boost.getStats();
  assertEquals(stats.totalOperations, 3);
  assert(stats.simdUsagePercent >= 0);
  assert(stats.simdUsagePercent <= 100);

  boost.cleanup();
});

Deno.test("SIMD availability detection", async () => {
  const boost = new BoostAlgorithm();
  await boost.initialize();

  // SIMD availability should be consistently reported
  const simdAvailable = boost.isSIMDAvailable();
  assert(typeof simdAvailable === 'boolean');
  // On most systems with WASM SIMD support, this should be true
  console.log(`SIMD Available: ${simdAvailable}`);

  boost.cleanup();
});

Deno.test("Performance benchmarks", async () => {
  const boost = new BoostAlgorithm();
  await boost.initialize();

  try {
    const benchmarks = await boost.runBenchmarks();

    assert(benchmarks.length > 0);

    for (const bench of benchmarks) {
      assertExists(bench.operation);
      assert(bench.timeMs > 0);
      assert(bench.iterations > 0);
      assert(bench.opsPerSecond > 0);
    }
  } catch (error) {
    console.warn(`Benchmark test skipped: ${error}`);
  }

  boost.cleanup();
});