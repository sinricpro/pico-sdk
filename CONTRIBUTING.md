# Contributing to SinricPro Pico SDK

Thank you for your interest in contributing! This document provides guidelines for contributing to the project.

## Development Setup

### Prerequisites

- Raspberry Pi Pico W or Pico 2 W (for testing)
- pico-sdk 2.2.0 or later
- CMake 3.13+
- ARM GCC toolchain (`gcc-arm-none-eabi`)
- Git

### Setting Up Development Environment

1. **Clone the repository**
   ```bash
   git clone https://github.com/sinricpro/pico-sdk.git
   cd pico-sdk
   ```

2. **Initialize submodules**
   ```bash
   git submodule add https://github.com/DaveGamble/cJSON.git lib/cJSON
   git submodule update --init --recursive
   ```

3. **Set pico-sdk path**
   ```bash
   export PICO_SDK_PATH=/path/to/pico-sdk
   ```

4. **Build**
   ```bash
   cmake -B build -DPICO_BOARD=pico_w
   cmake --build build -j$(nproc)
   ```

## Code Style

### C Code Guidelines

- **Standard**: C11
- **Indentation**: 4 spaces (no tabs)
- **Line length**: 100 characters maximum
- **Naming conventions**:
  - Functions: `sinricpro_module_action()` (snake_case)
  - Types: `sinricpro_type_t` (snake_case with `_t` suffix)
  - Constants: `SINRICPRO_CONSTANT` (UPPER_SNAKE_CASE)
  - Macros: `SINRICPRO_MACRO` (UPPER_SNAKE_CASE)

### File Organization

- **Headers**: `include/sinricpro/`
- **Implementation**: `src/`
- **Examples**: `examples/`
- **Tests**: `tests/` (future)

### Documentation

- Use Doxygen-style comments for public APIs
- Include usage examples in header comments
- Document all function parameters and return values

Example:
```c
/**
 * @brief Send a power state event
 *
 * @param device Device pointer
 * @param state Power state (true=ON, false=OFF)
 * @return true if event sent successfully
 */
bool sinricpro_switch_send_power_state_event(sinricpro_switch_t *device, bool state);
```

## Adding New Device Types

1. **Create capability files** (if needed)
   - Header: `include/sinricpro/capabilities/capability_name.h`
   - Implementation: `src/capabilities/capability_name.c`

2. **Create device files**
   - Header: `include/sinricpro/sinricpro_device_name.h`
   - Implementation: `src/devices/sinricpro_device_name.c`

3. **Add to CMakeLists.txt**
   ```cmake
   # In capabilities section
   src/capabilities/capability_name.c

   # In devices section
   src/devices/sinricpro_device_name.c
   ```

4. **Create example**
   - Directory: `examples/device_name/`
   - Files: `CMakeLists.txt`, `main.c`
   - Add to main `CMakeLists.txt` examples section

5. **Add device type constant**
   - Update `sinricpro_device.h` with new device type

6. **Update README.md**
   - Add to supported devices list
   - Add to examples list

## Testing

### Local Testing

Before submitting a PR:

1. **Build all examples**
   ```bash
   cmake --build build -j$(nproc)
   ```

2. **Check for warnings**
   - Code should compile without warnings
   - Use `-Wall -Wextra` flags

3. **Test on hardware**
   - Flash example to Pico W
   - Verify functionality with SinricPro portal
   - Test voice commands with Alexa/Google

### CI/CD

GitHub Actions will automatically:
- Build all examples on push/PR
- Run CodeQL security analysis
- Upload build artifacts

## Pull Request Process

1. **Fork the repository**

2. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Make your changes**
   - Follow code style guidelines
   - Add/update documentation
   - Create examples if adding new features

4. **Test thoroughly**
   - Build all examples
   - Test on hardware
   - Check for memory leaks

5. **Commit with clear messages**
   ```bash
   git commit -m "feat: add support for XYZ device"
   ```

   Use conventional commit format:
   - `feat:` - New feature
   - `fix:` - Bug fix
   - `docs:` - Documentation changes
   - `refactor:` - Code refactoring
   - `test:` - Adding tests
   - `chore:` - Build/tooling changes

6. **Push to your fork**
   ```bash
   git push origin feature/your-feature-name
   ```

7. **Create Pull Request**
   - Describe what changes you made
   - Reference any related issues
   - Include screenshots/videos if applicable
   - List tested hardware configurations

## Reporting Issues

When reporting bugs, please include:

- **Hardware**: Pico W or Pico 2 W
- **SDK version**: pico-sdk version used
- **SinricPro SDK version**: commit hash or release version
- **Steps to reproduce**
- **Expected behavior**
- **Actual behavior**
- **Serial output/logs** (if applicable)
- **Code snippet** (minimal reproducible example)

## Feature Requests

For feature requests:
- Check existing issues first
- Describe the use case
- Explain why it would be useful
- Provide examples if possible

## Code Review

All submissions require code review. We'll check for:
- Code quality and style
- Documentation completeness
- Test coverage
- Performance implications
- Security considerations

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Questions?

- Open an issue for questions
- Check existing issues and PRs
- Review the main README.md

Thank you for contributing! 🎉
