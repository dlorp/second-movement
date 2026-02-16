# Code Linting with clang-tidy

second-movement uses **clang-tidy** for static analysis and code quality.

## Quick Start

### Install clang-tidy

```bash
# macOS
brew install llvm

# Verify installation
clang-tidy --version
```

### Run locally

```bash
# Check all C files
./scripts/lint.sh

# Check specific file
./scripts/lint.sh watch-faces/complication/comms_rx.c

# Check specific directory
./scripts/lint.sh lib/fesk_tx
```

## What It Checks

### Security
- **Buffer overflows** - Bounds checks, array access validation
- **Integer overflows** - Counter overflow, arithmetic safety
- **Use-after-free** - Resource cleanup, pointer safety
- **CERT C violations** - Security coding standard compliance

### Code Quality
- **Readability** - Naming conventions, function complexity
- **Performance** - Inefficient patterns, unnecessary copies
- **Portability** - Platform-specific issues, type safety
- **Bugs** - Common C pitfalls, logic errors

## Configuration

`.clang-tidy` contains the full configuration:
- Enabled checks: clang-analyzer, bugprone, cert, readability, performance
- Naming conventions: `lower_case` functions/variables, `UPPER_CASE` constants
- Embedded C focus: Buffer safety, integer safety, resource management

## CI/CD

GitHub Actions automatically runs clang-tidy on every PR:
- Workflow: `.github/workflows/clang-tidy.yml`
- Triggers: Changes to `.c` or `.h` files
- **Non-blocking**: Warnings are informational, don't fail build

## Workflow

### Before Committing (Recommended)

```bash
# 1. Write code
vim watch-faces/complication/my_face.c

# 2. Run linter
./scripts/lint.sh watch-faces/complication/my_face.c

# 3. Fix critical warnings (buffer overflows, security)

# 4. Commit
git add -A
git commit -m "feat: add my_face"

# 5. Push (CI will re-run clang-tidy)
git push
```

### Interpreting Warnings

**Fix immediately:**
- 🔴 **Buffer overflows** (`bugprone-sizeof-expression`, `cert-arr30-c`)
- 🔴 **Integer overflows** (`cert-int30-c`, `bugprone-integer-division`)
- 🔴 **Use-after-free** (`clang-analyzer-core.NullDereference`)

**Fix when convenient:**
- 🟡 **Readability** (`readability-function-size`, `readability-identifier-naming`)
- 🟡 **Performance** (`performance-unnecessary-copy-initialization`)

**Can ignore (usually):**
- ⚪ **Magic numbers** (disabled by default for embedded)
- ⚪ **Function complexity** (some embedded code is inherently complex)

## Example Output

```
watch-faces/complication/comms_rx.c:285:13: warning: 
Value stored to 'bit_buffer' is never read [clang-analyzer-deadcode.DeadStores]
            state->rx_state.bit_buffer = 0;
            ^

lib/fesk_tx/fesk_tx.c:142:5: warning:
Potential buffer overflow [cert-arr30-c]
    memcpy(buffer, data, len);
    ^
```

## Disabling Specific Warnings

If a warning is a false positive, disable it locally:

```c
// NOLINTNEXTLINE(cert-arr30-c)
memcpy(buffer, data, len);  // Safe: len validated above
```

Or disable for entire file:

```c
// At top of file:
// NOLINTBEGIN(readability-function-size)
void very_large_function() {
    // ...
}
// NOLINTEND(readability-function-size)
```

## Common False Positives (Embedded C)

### Hardware Registers
```c
// Accessing hardware registers (common in embedded)
*(volatile uint32_t *)0x40000000 = value;  // May trigger warnings
```

Disable with:
```c
// NOLINTNEXTLINE(performance-no-int-to-ptr)
*(volatile uint32_t *)0x40000000 = value;
```

### Interrupt Handlers
```c
// Variables modified in ISRs need volatile
volatile bool flag = false;  // May trigger unused-variable warnings
```

### Packed Structures
```c
// Packed structs for hardware/protocol definitions
typedef struct __attribute__((packed)) {  // May trigger portability warnings
    uint8_t header;
    uint32_t data;
} packet_t;
```

## References

- **clang-tidy docs**: https://clang.llvm.org/extra/clang-tidy/
- **CERT C Coding Standard**: https://wiki.sei.cmu.edu/confluence/display/c/SEI+CERT+C+Coding+Standard
- **Embedded C best practices**: https://embeddedartistry.com/

## Questions?

- Check `.clang-tidy` for current configuration
- Run `./scripts/lint.sh --help` for usage
- See GitHub Actions logs for CI results
