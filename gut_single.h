/*
 * Gut - Game UI Toolkit
 * Single-header version (auto-generated)
 * 
 * Usage:
 *   #define GUT_IMPLEMENTATION
 *   #include "gut_single.h"
 * 
 * In other files, just:
 *   #include "gut_single.h"
 */

#ifndef GUT_SINGLE_HPP
#define GUT_SINGLE_HPP

// Standard library includes
#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <sstream>
#include <stack>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

// For single-header, we always use static linkage
#undef GUT_API
#define GUT_API


// ============================================================================
// DECLARATIONS
// ============================================================================


// --- gut/Export.h ---


// Platform detection - use ifndef to allow CMake definitions
#if defined(_WIN32) || defined(_WIN64)
    #ifndef GUT_PLATFORM_WINDOWS
        #define GUT_PLATFORM_WINDOWS
    #endif
#elif defined(__APPLE__)
    #ifndef GUT_PLATFORM_MACOS
        #define GUT_PLATFORM_MACOS
    #endif
#elif defined(__linux__)
    #ifndef GUT_PLATFORM_LINUX
        #define GUT_PLATFORM_LINUX
    #endif
#endif

// Export/import macros
#ifdef GUT_SHARED
    #ifdef GUT_PLATFORM_WINDOWS
        #ifdef GUT_BUILDING
            #define GUT_API __declspec(dllexport)
        #else
            #define GUT_API __declspec(dllimport)
        #endif
    #else
        #define GUT_API __attribute__((visibility("default")))
    #endif
#else
    #define GUT_API
#endif

// Disable copy
#define GUT_NONCOPYABLE(TypeName) \
    TypeName(const TypeName&) = delete; \
    TypeName& operator=(const TypeName&) = delete;

// Disable move
#define GUT_NONMOVABLE(TypeName) \
    TypeName(TypeName&&) = delete; \
    TypeName& operator=(TypeName&&) = delete;


// --- gut/Types.h ---


#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <optional>
#include <variant>
#include <memory>
#include <vector>
#include <span>

namespace gut {

// Integer types
using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

// Float types
using f32 = float;
using f64 = double;

// Size type
using usize = std::size_t;
using isize = std::ptrdiff_t;

// String types
using String = std::string;
using StringView = std::string_view;

// Forward declarations
class Object;
class Element;
class Visual;
class Panel;
class Context;

// 2D Point
template<typename T>
struct Point {
    T x{};
    T y{};
    
    constexpr Point() = default;
    constexpr Point(T x_, T y_) : x(x_), y(y_) {}
    
    constexpr Point operator+(const Point& other) const { return {x + other.x, y + other.y}; }
    constexpr Point operator-(const Point& other) const { return {x - other.x, y - other.y}; }
    constexpr Point operator*(T scalar) const { return {x * scalar, y * scalar}; }
    constexpr Point operator/(T scalar) const { return {x / scalar, y / scalar}; }
    
    constexpr bool operator==(const Point&) const = default;
};

using Point2f = Point<f32>;
using Point2i = Point<i32>;

// 2D Size
template<typename T>
struct Size {
    T width{};
    T height{};
    
    constexpr Size() = default;
    constexpr Size(T w, T h) : width(w), height(h) {}
    
    constexpr bool operator==(const Size&) const = default;
    
    constexpr bool isEmpty() const { return width <= 0 || height <= 0; }
};

using Size2f = Size<f32>;
using Size2i = Size<i32>;

// Rectangle
template<typename T>
struct Rect {
    T x{};
    T y{};
    T width{};
    T height{};
    
    constexpr Rect() = default;
    constexpr Rect(T x_, T y_, T w, T h) : x(x_), y(y_), width(w), height(h) {}
    constexpr Rect(Point<T> pos, Size<T> size) : x(pos.x), y(pos.y), width(size.width), height(size.height) {}
    
    constexpr Point<T> position() const { return {x, y}; }
    constexpr Size<T> size() const { return {width, height}; }
    
    constexpr T left() const { return x; }
    constexpr T top() const { return y; }
    constexpr T right() const { return x + width; }
    constexpr T bottom() const { return y + height; }
    
    constexpr Point<T> topLeft() const { return {x, y}; }
    constexpr Point<T> topRight() const { return {x + width, y}; }
    constexpr Point<T> bottomLeft() const { return {x, y + height}; }
    constexpr Point<T> bottomRight() const { return {x + width, y + height}; }
    constexpr Point<T> center() const { return {x + width / 2, y + height / 2}; }
    
    constexpr bool contains(Point<T> p) const {
        return p.x >= x && p.x < x + width && p.y >= y && p.y < y + height;
    }
    
    constexpr bool contains(const Rect& other) const {
        return other.x >= x && other.right() <= right() && 
               other.y >= y && other.bottom() <= bottom();
    }
    
    constexpr bool intersects(const Rect& other) const {
        return x < other.right() && right() > other.x &&
               y < other.bottom() && bottom() > other.y;
    }
    
    constexpr Rect intersection(const Rect& other) const {
        T l = std::max(x, other.x);
        T t = std::max(y, other.y);
        T r = std::min(right(), other.right());
        T b = std::min(bottom(), other.bottom());
        if (l < r && t < b) {
            return {l, t, r - l, b - t};
        }
        return {};
    }
    
    constexpr Rect united(const Rect& other) const {
        T l = std::min(x, other.x);
        T t = std::min(y, other.y);
        T r = std::max(right(), other.right());
        T b = std::max(bottom(), other.bottom());
        return {l, t, r - l, b - t};
    }
    
    constexpr bool isEmpty() const { return width <= 0 || height <= 0; }
    constexpr bool operator==(const Rect&) const = default;
};

using Rectf = Rect<f32>;
using Recti = Rect<i32>;

// Thickness (margins, padding, borders)
struct Thickness {
    f32 left{};
    f32 top{};
    f32 right{};
    f32 bottom{};
    
    constexpr Thickness() = default;
    constexpr Thickness(f32 uniform) : left(uniform), top(uniform), right(uniform), bottom(uniform) {}
    constexpr Thickness(f32 horizontal, f32 vertical) : left(horizontal), top(vertical), right(horizontal), bottom(vertical) {}
    constexpr Thickness(f32 l, f32 t, f32 r, f32 b) : left(l), top(t), right(r), bottom(b) {}
    
    constexpr f32 horizontalSum() const { return left + right; }
    constexpr f32 verticalSum() const { return top + bottom; }
    
    constexpr bool operator==(const Thickness&) const = default;
};

// Color (RGBA, 0-1 range)
struct Color {
    f32 r{0.0f};
    f32 g{0.0f};
    f32 b{0.0f};
    f32 a{1.0f};
    
    constexpr Color() = default;
    constexpr Color(f32 r_, f32 g_, f32 b_, f32 a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}
    
    // From 0-255 range
    static constexpr Color fromRgba8(u8 r, u8 g, u8 b, u8 a = 255) {
        return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
    }
    
    // From hex (0xRRGGBB or 0xRRGGBBAA)
    static constexpr Color fromHex(u32 hex) {
        if (hex > 0xFFFFFF) {
            // Has alpha
            return fromRgba8(
                (hex >> 24) & 0xFF,
                (hex >> 16) & 0xFF,
                (hex >> 8) & 0xFF,
                hex & 0xFF
            );
        }
        return fromRgba8(
            (hex >> 16) & 0xFF,
            (hex >> 8) & 0xFF,
            hex & 0xFF
        );
    }
    
    constexpr u32 toRgba8() const {
        return (static_cast<u32>(r * 255) << 24) |
               (static_cast<u32>(g * 255) << 16) |
               (static_cast<u32>(b * 255) << 8) |
               static_cast<u32>(a * 255);
    }
    
    constexpr Color withAlpha(f32 alpha) const { return {r, g, b, alpha}; }
    
    constexpr Color operator*(f32 scalar) const { return {r * scalar, g * scalar, b * scalar, a}; }
    constexpr Color operator+(const Color& other) const { return {r + other.r, g + other.g, b + other.b, a + other.a}; }
    
    constexpr bool operator==(const Color&) const = default;
    
    // Common colors
    static constexpr Color transparent() { return {0, 0, 0, 0}; }
    static constexpr Color black() { return {0, 0, 0, 1}; }
    static constexpr Color white() { return {1, 1, 1, 1}; }
    static constexpr Color red() { return {1, 0, 0, 1}; }
    static constexpr Color green() { return {0, 1, 0, 1}; }
    static constexpr Color blue() { return {0, 0, 1, 1}; }
    static constexpr Color yellow() { return {1, 1, 0, 1}; }
    static constexpr Color cyan() { return {0, 1, 1, 1}; }
    static constexpr Color magenta() { return {1, 0, 1, 1}; }
    static constexpr Color gray() { return {0.5f, 0.5f, 0.5f, 1}; }
};

// Alignment enums
enum class HorizontalAlignment : u8 {
    Left,
    Center,
    Right,
    Stretch
};

enum class VerticalAlignment : u8 {
    Top,
    Center,
    Bottom,
    Stretch
};

// Orientation
enum class Orientation : u8 {
    Horizontal,
    Vertical
};

// Visibility
enum class Visibility : u8 {
    Visible,
    Hidden,     // Takes space but not rendered
    Collapsed   // No space, not rendered
};

// Cursor types
enum class CursorType : u8 {
    Arrow,
    IBeam,
    Hand,
    SizeNS,
    SizeWE,
    SizeNWSE,
    SizeNESW,
    Move,
    Wait,
    Forbidden
};

} // namespace gut


// --- gut/core/RefCounted.h ---


#include <atomic>
#include <type_traits>

namespace gut {

/**
 * @brief Intrusive reference-counted base class.
 * 
 * Provides thread-safe reference counting. Derive from this class
 * and use Ref<T> smart pointers for automatic lifetime management.
 */
class GUT_API RefCounted {
public:
    RefCounted() = default;
    virtual ~RefCounted() = default;
    
    GUT_NONCOPYABLE(RefCounted)
    
    void addRef() const noexcept {
        m_refCount.fetch_add(1, std::memory_order_relaxed);
    }
    
    void release() const noexcept {
        if (m_refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete this;
        }
    }
    
    u32 refCount() const noexcept {
        return m_refCount.load(std::memory_order_relaxed);
    }

private:
    mutable std::atomic<u32> m_refCount{0};
};

} // namespace gut


// --- gut/core/Ptr.h ---


#include <utility>
#include <concepts>

namespace gut {

/**
 * @brief Intrusive reference-counting smart pointer.
 * 
 * Works with types derived from RefCounted.
 */
template<typename T>
class Ref {
public:
    constexpr Ref() noexcept : m_ptr(nullptr) {}
    constexpr Ref(std::nullptr_t) noexcept : m_ptr(nullptr) {}
    
    explicit Ref(T* ptr) noexcept : m_ptr(ptr) {
        if (m_ptr) m_ptr->addRef();
    }
    
    Ref(const Ref& other) noexcept : m_ptr(other.m_ptr) {
        if (m_ptr) m_ptr->addRef();
    }
    
    Ref(Ref&& other) noexcept : m_ptr(other.m_ptr) {
        other.m_ptr = nullptr;
    }
    
    template<typename U>
        requires std::derived_from<U, T>
    Ref(const Ref<U>& other) noexcept : m_ptr(other.get()) {
        if (m_ptr) m_ptr->addRef();
    }
    
    template<typename U>
        requires std::derived_from<U, T>
    Ref(Ref<U>&& other) noexcept : m_ptr(other.get()) {
        other.m_ptr = nullptr;
    }
    
    ~Ref() {
        if (m_ptr) m_ptr->release();
    }
    
    Ref& operator=(const Ref& other) noexcept {
        if (this != &other) {
            if (other.m_ptr) other.m_ptr->addRef();
            if (m_ptr) m_ptr->release();
            m_ptr = other.m_ptr;
        }
        return *this;
    }
    
    Ref& operator=(Ref&& other) noexcept {
        if (this != &other) {
            if (m_ptr) m_ptr->release();
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
        }
        return *this;
    }
    
    Ref& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }
    
    void reset() noexcept {
        if (m_ptr) {
            m_ptr->release();
            m_ptr = nullptr;
        }
    }
    
    void reset(T* ptr) noexcept {
        if (ptr) ptr->addRef();
        if (m_ptr) m_ptr->release();
        m_ptr = ptr;
    }
    
    T* get() const noexcept { return m_ptr; }
    T* operator->() const noexcept { return m_ptr; }
    T& operator*() const noexcept { return *m_ptr; }
    
    explicit operator bool() const noexcept { return m_ptr != nullptr; }
    
    bool operator==(const Ref& other) const noexcept { return m_ptr == other.m_ptr; }
    bool operator!=(const Ref& other) const noexcept { return m_ptr != other.m_ptr; }
    bool operator==(std::nullptr_t) const noexcept { return m_ptr == nullptr; }
    bool operator!=(std::nullptr_t) const noexcept { return m_ptr != nullptr; }
    
    template<typename U>
    Ref<U> as() const {
        return Ref<U>(static_cast<U*>(m_ptr));
    }
    
    template<typename U>
    Ref<U> dynamicAs() const {
        return Ref<U>(dynamic_cast<U*>(m_ptr));
    }

private:
    template<typename U> friend class Ref;
    T* m_ptr;
};

/**
 * @brief Create a reference-counted object.
 */
template<typename T, typename... Args>
    requires std::derived_from<T, RefCounted>
Ref<T> make(Args&&... args) {
    return Ref<T>(new T(std::forward<Args>(args)...));
}

/**
 * @brief Alias for make - creates a reference-counted object.
 */
template<typename T, typename... Args>
    requires std::derived_from<T, RefCounted>
Ref<T> makeRef(Args&&... args) {
    return Ref<T>(new T(std::forward<Args>(args)...));
}

/**
 * @brief Weak reference to a RefCounted object (non-owning).
 * 
 * Note: This is a simple non-owning pointer. For proper weak references
 * with validity tracking, use WeakRef with Control Block (future enhancement).
 */
template<typename T>
class WeakPtr {
public:
    constexpr WeakPtr() noexcept : m_ptr(nullptr) {}
    constexpr WeakPtr(std::nullptr_t) noexcept : m_ptr(nullptr) {}
    explicit WeakPtr(T* ptr) noexcept : m_ptr(ptr) {}
    WeakPtr(const Ref<T>& ref) noexcept : m_ptr(ref.get()) {}
    
    T* get() const noexcept { return m_ptr; }
    T* operator->() const noexcept { return m_ptr; }
    T& operator*() const noexcept { return *m_ptr; }
    
    explicit operator bool() const noexcept { return m_ptr != nullptr; }
    
    Ref<T> lock() const { return Ref<T>(m_ptr); }
    
    void reset() noexcept { m_ptr = nullptr; }

private:
    T* m_ptr;
};

} // namespace gut


// --- gut/core/Signal.h ---


#include <functional>
#include <vector>
#include <algorithm>

namespace gut {

/**
 * @brief Connection handle for signal-slot connections.
 */
class GUT_API Connection {
public:
    Connection() : m_id(0) {}
    explicit Connection(u64 id) : m_id(id) {}
    
    bool isValid() const { return m_id != 0; }
    u64 id() const { return m_id; }
    
    bool operator==(const Connection& other) const { return m_id == other.m_id; }
    bool operator!=(const Connection& other) const { return m_id != other.m_id; }

private:
    u64 m_id;
};

/**
 * @brief Type-erased signal base class.
 */
class GUT_API SignalBase {
public:
    virtual ~SignalBase() = default;
    virtual void disconnect(Connection conn) = 0;
    virtual void disconnectAll() = 0;
};

/**
 * @brief Signal class for event notification (observer pattern).
 * 
 * Signals allow connecting callable handlers that will be invoked
 * when the signal is emitted. Thread-safe for connecting/disconnecting
 * but emission should happen on a single thread.
 * 
 * @tparam Args The argument types passed to connected handlers.
 */
template<typename... Args>
class Signal : public SignalBase {
public:
    using Handler = std::function<void(Args...)>;
    
    Signal() = default;
    ~Signal() override = default;
    
    GUT_NONCOPYABLE(Signal)
    Signal(Signal&&) = default;
    Signal& operator=(Signal&&) = default;
    
    /**
     * @brief Connect a handler to this signal.
     * @return Connection handle that can be used to disconnect.
     */
    Connection connect(Handler handler) {
        u64 id = ++m_nextId;
        m_handlers.push_back({id, std::move(handler)});
        return Connection(id);
    }
    
    /**
     * @brief Connect a member function to this signal.
     */
    template<typename T>
    Connection connect(T* obj, void (T::*method)(Args...)) {
        return connect([obj, method](Args... args) {
            (obj->*method)(std::forward<Args>(args)...);
        });
    }
    
    /**
     * @brief Disconnect a handler by connection handle.
     */
    void disconnect(Connection conn) override {
        auto it = std::find_if(m_handlers.begin(), m_handlers.end(),
            [&](const Slot& slot) { return slot.id == conn.id(); });
        if (it != m_handlers.end()) {
            m_handlers.erase(it);
        }
    }
    
    /**
     * @brief Disconnect all handlers.
     */
    void disconnectAll() override {
        m_handlers.clear();
    }
    
    /**
     * @brief Emit the signal, invoking all connected handlers.
     */
    void emit(Args... args) const {
        // Copy handlers in case a handler disconnects during iteration
        auto handlers = m_handlers;
        for (const auto& slot : handlers) {
            slot.handler(args...);
        }
    }
    
    /**
     * @brief Emit the signal using operator().
     */
    void operator()(Args... args) const {
        emit(std::forward<Args>(args)...);
    }
    
    /**
     * @brief Check if any handlers are connected.
     */
    bool hasConnections() const {
        return !m_handlers.empty();
    }
    
    /**
     * @brief Get the number of connected handlers.
     */
    usize connectionCount() const {
        return m_handlers.size();
    }

private:
    struct Slot {
        u64 id;
        Handler handler;
    };
    
    std::vector<Slot> m_handlers;
    u64 m_nextId{0};
};

/**
 * @brief RAII connection manager that disconnects on destruction.
 */
class GUT_API ScopedConnection {
public:
    ScopedConnection() = default;
    ScopedConnection(SignalBase& signal, Connection conn)
        : m_signal(&signal), m_connection(conn) {}
    
    ~ScopedConnection() {
        disconnect();
    }
    
    ScopedConnection(ScopedConnection&& other) noexcept
        : m_signal(other.m_signal), m_connection(other.m_connection) {
        other.m_signal = nullptr;
        other.m_connection = Connection();
    }
    
    ScopedConnection& operator=(ScopedConnection&& other) noexcept {
        if (this != &other) {
            disconnect();
            m_signal = other.m_signal;
            m_connection = other.m_connection;
            other.m_signal = nullptr;
            other.m_connection = Connection();
        }
        return *this;
    }
    
    GUT_NONCOPYABLE(ScopedConnection)
    
    void disconnect() {
        if (m_signal && m_connection.isValid()) {
            m_signal->disconnect(m_connection);
            m_connection = Connection();
        }
    }
    
    Connection connection() const { return m_connection; }

private:
    SignalBase* m_signal{nullptr};
    Connection m_connection;
};

} // namespace gut


// --- gut/core/Property.h ---


#include <any>
#include <functional>
#include <unordered_map>
#include <string>
#include <typeindex>

namespace gut {

// Forward declarations
class Object;

/**
 * @brief Property identifier.
 * 
 * Properties are identified by a unique ID and have associated metadata.
 */
class GUT_API PropertyId {
public:
    PropertyId() : m_id(0) {}
    explicit PropertyId(u32 id) : m_id(id) {}
    
    u32 id() const { return m_id; }
    bool isValid() const { return m_id != 0; }
    
    bool operator==(const PropertyId& other) const { return m_id == other.m_id; }
    bool operator!=(const PropertyId& other) const { return m_id != other.m_id; }
    bool operator<(const PropertyId& other) const { return m_id < other.m_id; }

private:
    u32 m_id;
};

} // namespace gut

// Hash for PropertyId
template<>
struct std::hash<gut::PropertyId> {
    std::size_t operator()(const gut::PropertyId& p) const noexcept {
        return std::hash<gut::u32>{}(p.id());
    }
};

namespace gut {

/**
 * @brief Property metadata and registration.
 */
class GUT_API PropertyMeta {
public:
    enum class Flags : u32 {
        None            = 0,
        Inherits        = 1 << 0,  // Value inherits from parent
        AffectsLayout   = 1 << 1,  // Changing value triggers layout
        AffectsRender   = 1 << 2,  // Changing value triggers render
        Animatable      = 1 << 3,  // Value can be animated
        Styleable       = 1 << 4,  // Value can be set via styles
    };
    
    PropertyMeta(
        StringView name,
        std::type_index valueType,
        std::any defaultValue,
        Flags flags = Flags::None
    ) : m_name(name),
        m_valueType(valueType),
        m_defaultValue(std::move(defaultValue)),
        m_flags(flags) {}
    
    StringView name() const { return m_name; }
    std::type_index valueType() const { return m_valueType; }
    const std::any& defaultValue() const { return m_defaultValue; }
    Flags flags() const { return m_flags; }
    
    bool inherits() const { return hasFlag(Flags::Inherits); }
    bool affectsLayout() const { return hasFlag(Flags::AffectsLayout); }
    bool affectsRender() const { return hasFlag(Flags::AffectsRender); }
    bool isAnimatable() const { return hasFlag(Flags::Animatable); }
    bool isStyleable() const { return hasFlag(Flags::Styleable); }

private:
    bool hasFlag(Flags f) const {
        return (static_cast<u32>(m_flags) & static_cast<u32>(f)) != 0;
    }
    
    String m_name;
    std::type_index m_valueType;
    std::any m_defaultValue;
    Flags m_flags;
};

inline PropertyMeta::Flags operator|(PropertyMeta::Flags a, PropertyMeta::Flags b) {
    return static_cast<PropertyMeta::Flags>(static_cast<u32>(a) | static_cast<u32>(b));
}

/**
 * @brief Property registry - central registry of all property definitions.
 */
class GUT_API PropertyRegistry {
public:
    static PropertyRegistry& instance();
    
    /**
     * @brief Register a new property.
     * @return The property ID for the registered property.
     */
    template<typename T>
    PropertyId registerProperty(
        StringView name,
        T defaultValue,
        PropertyMeta::Flags flags = PropertyMeta::Flags::None
    ) {
        PropertyId id(++m_nextId);
        m_properties.emplace(id, PropertyMeta(
            name,
            typeid(T),
            std::any(std::move(defaultValue)),
            flags
        ));
        m_nameToId[String(name)] = id;
        return id;
    }
    
    /**
     * @brief Get property metadata by ID.
     */
    const PropertyMeta* getMeta(PropertyId id) const;
    
    /**
     * @brief Get property ID by name.
     */
    PropertyId findByName(StringView name) const;

private:
    PropertyRegistry() = default;
    
    std::unordered_map<PropertyId, PropertyMeta> m_properties;
    std::unordered_map<String, PropertyId> m_nameToId;
    u32 m_nextId{0};
};

/**
 * @brief Property value with change notification.
 * 
 * A Property<T> holds a value and emits a signal when it changes.
 * Properties can be bound to other properties for automatic updates.
 */
template<typename T>
class Property {
public:
    using ValueType = T;
    
    Property() : m_value{} {}
    explicit Property(T value) : m_value(std::move(value)) {}
    
    /**
     * @brief Get the current value.
     */
    const T& get() const { return m_value; }
    
    /**
     * @brief Get the current value (implicit conversion).
     */
    operator const T&() const { return m_value; }
    
    /**
     * @brief Set the value (emits changed signal if different).
     */
    void set(T value) {
        if (m_value != value) {
            T oldValue = std::move(m_value);
            m_value = std::move(value);
            m_changed.emit(m_value, oldValue);
        }
    }
    
    /**
     * @brief Set the value using assignment.
     */
    Property& operator=(T value) {
        set(std::move(value));
        return *this;
    }
    
    /**
     * @brief Signal emitted when the value changes.
     * @param newValue The new value.
     * @param oldValue The previous value.
     */
    Signal<const T&, const T&>& changed() { return m_changed; }
    
    /**
     * @brief Bind this property to another property (one-way).
     * The value of this property will be updated when the source changes.
     */
    void bind(Property<T>& source) {
        m_bindingConnection = source.changed().connect([this](const T& newVal, const T&) {
            set(newVal);
        });
        set(source.get());
    }
    
    /**
     * @brief Remove any binding.
     */
    void unbind() {
        m_bindingConnection.disconnect();
    }

private:
    T m_value;
    Signal<const T&, const T&> m_changed;
    ScopedConnection m_bindingConnection;
};

/**
 * @brief Helper macro to define a property with getter/setter.
 */
#define GUT_PROPERTY(Type, Name, DefaultValue) \
private: \
    gut::Property<Type> m_##Name{DefaultValue}; \
public: \
    const Type& Name() const { return m_##Name.get(); } \
    void set##Name(Type value) { m_##Name.set(std::move(value)); } \
    gut::Property<Type>& Name##Property() { return m_##Name; }

/**
 * @brief Helper macro to define a readonly property.
 */
#define GUT_PROPERTY_READONLY(Type, Name, DefaultValue) \
private: \
    gut::Property<Type> m_##Name{DefaultValue}; \
public: \
    const Type& Name() const { return m_##Name.get(); } \
protected: \
    void set##Name(Type value) { m_##Name.set(std::move(value)); } \
    gut::Property<Type>& Name##Property() { return m_##Name; }

} // namespace gut


// --- gut/core/Object.h ---


#include <typeinfo>
#include <typeindex>

namespace gut {

// Forward declarations
class Object;
class TypeInfo;

/**
 * @brief Runtime type information for gut objects.
 */
class GUT_API TypeInfo {
public:
    TypeInfo(const char* name, const TypeInfo* base, std::type_index stdType)
        : m_name(name), m_base(base), m_stdType(stdType) {}
    
    const char* name() const { return m_name; }
    const TypeInfo* base() const { return m_base; }
    std::type_index stdType() const { return m_stdType; }
    
    bool isA(const TypeInfo& other) const {
        const TypeInfo* current = this;
        while (current) {
            if (current->m_stdType == other.m_stdType) return true;
            current = current->m_base;
        }
        return false;
    }
    
    bool operator==(const TypeInfo& other) const { return m_stdType == other.m_stdType; }
    bool operator!=(const TypeInfo& other) const { return m_stdType != other.m_stdType; }

private:
    const char* m_name;
    const TypeInfo* m_base;
    std::type_index m_stdType;
};

// Macros for declaring type info
#define GUT_OBJECT(TypeName, BaseType) \
public: \
    static const gut::TypeInfo& staticTypeInfo() { \
        static gut::TypeInfo info(#TypeName, &BaseType::staticTypeInfo(), typeid(TypeName)); \
        return info; \
    } \
    const gut::TypeInfo& typeInfo() const override { return staticTypeInfo(); } \
private:

#define GUT_ROOT_OBJECT(TypeName) \
public: \
    static const gut::TypeInfo& staticTypeInfo() { \
        static gut::TypeInfo info(#TypeName, nullptr, typeid(TypeName)); \
        return info; \
    } \
    virtual const gut::TypeInfo& typeInfo() const { return staticTypeInfo(); } \
private:

/**
 * @brief Base class for all gut objects.
 * 
 * Provides:
 * - Intrusive reference counting
 * - Runtime type information
 * - Type-safe casting
 */
class GUT_API Object : public RefCounted {
    GUT_ROOT_OBJECT(Object)
    
public:
    Object() = default;
    ~Object() override = default;
    
    /**
     * @brief Check if this object is of the given type or derived from it.
     */
    template<typename T>
    bool isA() const {
        return typeInfo().isA(T::staticTypeInfo());
    }
    
    /**
     * @brief Cast to a derived type (returns nullptr if not compatible).
     */
    template<typename T>
    T* as() {
        return isA<T>() ? static_cast<T*>(this) : nullptr;
    }
    
    template<typename T>
    const T* as() const {
        return isA<T>() ? static_cast<const T*>(this) : nullptr;
    }
};

/**
 * @brief Type-safe cast from Ref<Object> to Ref<T>.
 */
template<typename T>
Ref<T> cast(const Ref<Object>& obj) {
    if (obj && obj->isA<T>()) {
        return Ref<T>(static_cast<T*>(obj.get()));
    }
    return nullptr;
}

} // namespace gut


// --- gut/render/Geometry.h ---


#include <vector>

namespace gut {

/**
 * @brief Path segment types.
 */
enum class PathSegmentType {
    MoveTo,
    LineTo,
    QuadTo,
    CubicTo,
    ArcTo,
    Close
};

/**
 * @brief Path segment data.
 */
struct GUT_API PathSegment {
    PathSegmentType type;
    Point2f points[3];  // Max 3 control points
    f32 param{0};       // Additional parameter (e.g., arc radius)
    
    static PathSegment moveTo(Point2f p) {
        PathSegment s; s.type = PathSegmentType::MoveTo; s.points[0] = p; return s;
    }
    static PathSegment lineTo(Point2f p) {
        PathSegment s; s.type = PathSegmentType::LineTo; s.points[0] = p; return s;
    }
    static PathSegment quadTo(Point2f control, Point2f end) {
        PathSegment s; s.type = PathSegmentType::QuadTo; s.points[0] = control; s.points[1] = end; return s;
    }
    static PathSegment cubicTo(Point2f c1, Point2f c2, Point2f end) {
        PathSegment s; s.type = PathSegmentType::CubicTo; s.points[0] = c1; s.points[1] = c2; s.points[2] = end; return s;
    }
    static PathSegment close() {
        PathSegment s; s.type = PathSegmentType::Close; return s;
    }
};

/**
 * @brief Fill rule for paths.
 */
enum class FillRule {
    EvenOdd,
    NonZero
};

/**
 * @brief 2D path geometry.
 */
class GUT_API Path : public Object {
    GUT_OBJECT(Path, Object)
    
public:
    Path() = default;
    ~Path() override = default;
    
    // -------------------------------------------------------------------------
    // Path construction
    // -------------------------------------------------------------------------
    
    void moveTo(f32 x, f32 y);
    void moveTo(Point2f p) { moveTo(p.x, p.y); }
    
    void lineTo(f32 x, f32 y);
    void lineTo(Point2f p) { lineTo(p.x, p.y); }
    
    void quadTo(f32 cx, f32 cy, f32 x, f32 y);
    void quadTo(Point2f control, Point2f end) { quadTo(control.x, control.y, end.x, end.y); }
    
    void cubicTo(f32 c1x, f32 c1y, f32 c2x, f32 c2y, f32 x, f32 y);
    void cubicTo(Point2f c1, Point2f c2, Point2f end) { cubicTo(c1.x, c1.y, c2.x, c2.y, end.x, end.y); }
    
    void arcTo(f32 rx, f32 ry, f32 rotation, bool largeArc, bool sweep, f32 x, f32 y);
    
    void close();
    
    void clear();
    
    // -------------------------------------------------------------------------
    // Shape helpers
    // -------------------------------------------------------------------------
    
    void addRect(Rectf rect);
    void addRoundedRect(Rectf rect, f32 cornerRadius);
    void addRoundedRect(Rectf rect, f32 topLeft, f32 topRight, f32 bottomRight, f32 bottomLeft);
    void addEllipse(Point2f center, f32 radiusX, f32 radiusY);
    void addCircle(Point2f center, f32 radius);
    
    // -------------------------------------------------------------------------
    // Properties
    // -------------------------------------------------------------------------
    
    const std::vector<PathSegment>& segments() const { return m_segments; }
    bool isEmpty() const { return m_segments.empty(); }
    
    FillRule fillRule() const { return m_fillRule; }
    void setFillRule(FillRule rule) { m_fillRule = rule; }
    
    /**
     * @brief Get the bounding box of the path.
     */
    Rectf bounds() const { return m_bounds; }
    
    /**
     * @brief Check if a point is inside the path.
     */
    bool contains(Point2f point) const;

    /**
     * @brief Flatten the path into sub-paths of line segments.
     *
     * Converts all curves (quad, cubic, arc) into polylines at the
     * given tolerance.  Each sub-path is a separate vector.
     * Closed sub-paths have their first point duplicated at the end.
     */
    std::vector<std::vector<Point2f>> flatten(f32 tolerance = 0.5f) const;

private:
    void updateBounds(Point2f point);
    
    std::vector<PathSegment> m_segments;
    Point2f m_currentPoint{};
    Rectf m_bounds{};
    bool m_boundsValid{false};
    FillRule m_fillRule{FillRule::EvenOdd};
};

} // namespace gut


// --- gut/render/Brush.h ---


#include <vector>

namespace gut {

/**
 * @brief Brush types for filling shapes.
 */
enum class BrushType {
    Solid,
    LinearGradient,
    RadialGradient,
    Texture
};

/**
 * @brief Gradient stop for gradient brushes.
 */
struct GUT_API GradientStop {
    f32 offset{0.0f};  // 0.0 to 1.0
    Color color;
    
    GradientStop() = default;
    GradientStop(f32 off, Color c) : offset(off), color(c) {}
};

/**
 * @brief Abstract brush for filling shapes.
 */
class GUT_API Brush : public Object {
    GUT_OBJECT(Brush, Object)
    
public:
    Brush() = default;
    ~Brush() override = default;
    
    virtual BrushType type() const = 0;
    
    f32 opacity() const { return m_opacity; }
    void setOpacity(f32 o) { m_opacity = o; }

private:
    f32 m_opacity{1.0f};
};

/**
 * @brief Solid color brush.
 */
class GUT_API SolidColorBrush : public Brush {
    GUT_OBJECT(SolidColorBrush, Brush)
    
public:
    SolidColorBrush() = default;
    explicit SolidColorBrush(Color color);
    ~SolidColorBrush() override = default;
    
    BrushType type() const override { return BrushType::Solid; }
    
    Color color() const { return m_color; }
    void setColor(Color c) { m_color = c; }

private:
    Color m_color{0, 0, 0, 1};
};

/**
 * @brief Linear gradient brush.
 */
class GUT_API LinearGradientBrush : public Brush {
    GUT_OBJECT(LinearGradientBrush, Brush)
    
public:
    LinearGradientBrush() = default;
    LinearGradientBrush(Point2f start, Point2f end, std::vector<GradientStop> stops);
    ~LinearGradientBrush() override = default;
    
    BrushType type() const override { return BrushType::LinearGradient; }
    
    Point2f startPoint() const { return m_startPoint; }
    void setStartPoint(Point2f p) { m_startPoint = p; }
    
    Point2f endPoint() const { return m_endPoint; }
    void setEndPoint(Point2f p) { m_endPoint = p; }
    
    void setStops(std::vector<GradientStop> stops) { m_stops = std::move(stops); }
    const std::vector<GradientStop>& stops() const { return m_stops; }

private:
    Point2f m_startPoint{0.0f, 0.0f};
    Point2f m_endPoint{1.0f, 1.0f};
    std::vector<GradientStop> m_stops;
};

/**
 * @brief Radial gradient brush.
 */
class GUT_API RadialGradientBrush : public Brush {
    GUT_OBJECT(RadialGradientBrush, Brush)
    
public:
    RadialGradientBrush() = default;
    RadialGradientBrush(Point2f center, f32 radius, std::vector<GradientStop> stops);
    ~RadialGradientBrush() override = default;
    
    BrushType type() const override { return BrushType::RadialGradient; }
    
    Point2f center() const { return m_center; }
    void setCenter(Point2f c) { m_center = c; }
    
    f32 radiusX() const { return m_radiusX; }
    void setRadiusX(f32 r) { m_radiusX = r; }
    
    f32 radiusY() const { return m_radiusY; }
    void setRadiusY(f32 r) { m_radiusY = r; }
    
    Point2f gradientOrigin() const { return m_gradientOrigin; }
    void setGradientOrigin(Point2f o) { m_gradientOrigin = o; }
    
    void setStops(std::vector<GradientStop> stops) { m_stops = std::move(stops); }
    const std::vector<GradientStop>& stops() const { return m_stops; }

private:
    Point2f m_center{0.5f, 0.5f};
    f32 m_radiusX{0.5f};
    f32 m_radiusY{0.5f};
    Point2f m_gradientOrigin{0.5f, 0.5f};
    std::vector<GradientStop> m_stops;
};

} // namespace gut


// --- gut/render/Pen.h ---



namespace gut {

/**
 * @brief Line cap style.
 */
enum class LineCap {
    Flat,
    Round,
    Square
};

/**
 * @brief Line join style.
 */
enum class LineJoin {
    Miter,
    Bevel,
    Round
};

/**
 * @brief Pen for stroking shapes.
 */
class GUT_API Pen : public Object {
    GUT_OBJECT(Pen, Object)
    
public:
    Pen() = default;
    explicit Pen(Color color, f32 thickness = 1.0f);
    explicit Pen(Ref<Brush> brush, f32 thickness = 1.0f);
    ~Pen() override = default;
    
    /**
     * @brief Get/set the brush used for stroking.
     */
    void setBrush(Ref<Brush> brush) { m_brush = std::move(brush); }
    Brush* brush() const { return m_brush.get(); }
    
    GUT_PROPERTY(f32, thickness, 1.0f)
    GUT_PROPERTY(LineCap, lineCap, LineCap::Flat)
    GUT_PROPERTY(LineJoin, lineJoin, LineJoin::Miter)
    GUT_PROPERTY(f32, miterLimit, 10.0f)
    
    /**
     * @brief Set dash pattern (empty = solid line).
     */
    void setDashPattern(std::vector<f32> pattern) { m_dashPattern = std::move(pattern); }
    const std::vector<f32>& dashPattern() const { return m_dashPattern; }
    
    GUT_PROPERTY(f32, dashOffset, 0.0f)

private:
    Ref<Brush> m_brush;
    std::vector<f32> m_dashPattern;
};

} // namespace gut


// --- gut/render/Texture.h ---



namespace gut {

/**
 * @brief Abstract texture resource.
 */
class GUT_API Texture : public Object {
    GUT_OBJECT(Texture, Object)
    
public:
    Texture() = default;
    ~Texture() override = default;
    
    /**
     * @brief Get the texture width in pixels.
     */
    virtual u32 width() const = 0;
    
    /**
     * @brief Get the texture height in pixels.
     */
    virtual u32 height() const = 0;
    
    /**
     * @brief Get size as Size2f.
     */
    Size2f size() const { return {static_cast<f32>(width()), static_cast<f32>(height())}; }
    
    /**
     * @brief Get the native texture handle (platform-specific).
     */
    virtual void* nativeHandle() const = 0;
};

} // namespace gut


// --- gut/render/RenderBackend.h ---


#include <span>

namespace gut {

// Forward declarations
class Texture;
struct TextLayout;

/**
 * @brief Vertex format for rendering.
 */
struct GUT_API Vertex {
    f32 x, y;           // Position
    f32 u, v;           // Texture coordinates
    u32 color;          // RGBA packed color
};

/**
 * @brief Text alignment.
 */
enum class TextAlignment {
    Left,
    Center,
    Right,
    Justify
};

/**
 * @brief Draw command type.
 */
enum class DrawCommandType {
    SetClip,
    ClearClip,
    DrawTriangles,
    DrawTexturedTriangles,
    DrawBackdropBlur
};

/**
 * @brief Draw command sent to the render backend.
 */
struct GUT_API DrawCommand {
    DrawCommandType type;
    u32 vertexOffset;
    u32 indexOffset;
    u32 indexCount;
    Rectf clipRect;
    f32 clipCornerRadius{0};
    Texture* texture{nullptr};
    
    // Backdrop blur fields (used by DrawBackdropBlur)
    Rectf blurRect;           // screen-space rect to blur
    f32 blurRadius{0};        // blur radius in logical pixels
    f32 blurCornerRadius{0};  // rounded corners for the blur region
    Color blurTint{Color::transparent()};  // tint overlay
};

/**
 * @brief Abstract render backend interface.
 * 
 * Implement this interface to integrate gut with your graphics API
 * (OpenGL, Vulkan, D3D11, D3D12, Metal, etc.)
 */
class GUT_API RenderBackend {
public:
    virtual ~RenderBackend() = default;
    
    // -------------------------------------------------------------------------
    // Lifecycle
    // -------------------------------------------------------------------------
    
    /**
     * @brief Initialize the backend.
     * @return true on success.
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Shutdown the backend.
     */
    virtual void shutdown() = 0;
    
    // -------------------------------------------------------------------------
    // Frame control
    // -------------------------------------------------------------------------
    
    /**
     * @brief Begin a new frame.
     * @param width Viewport width in pixels.
     * @param height Viewport height in pixels.
     * @param devicePixelRatio Device pixel ratio for HiDPI displays.
     */
    virtual void beginFrame(u32 width, u32 height, f32 devicePixelRatio) = 0;
    
    /**
     * @brief End the frame.
     */
    virtual void endFrame() = 0;
    
    // -------------------------------------------------------------------------
    // Rendering
    // -------------------------------------------------------------------------
    
    /**
     * @brief Submit draw commands for rendering.
     * @param vertices Vertex buffer.
     * @param indices Index buffer.
     * @param commands Draw commands.
     */
    virtual void render(
        std::span<const Vertex> vertices,
        std::span<const u32> indices,
        std::span<const DrawCommand> commands
    ) = 0;
    
    // -------------------------------------------------------------------------
    // Texture management
    // -------------------------------------------------------------------------
    
    /**
     * @brief Create a texture from RGBA pixel data.
     * @param width Texture width.
     * @param height Texture height.
     * @param pixels RGBA pixel data (4 bytes per pixel).
     * @return The created texture, or nullptr on failure.
     */
    virtual Ref<Texture> createTexture(u32 width, u32 height, const u8* pixels) = 0;
    
    /**
     * @brief Create a texture from a file.
     * @param path Path to the image file.
     * @return The created texture, or nullptr on failure.
     */
    virtual Ref<Texture> createTextureFromFile(StringView path) = 0;
    
    /**
     * @brief Update texture data.
     * @param texture The texture to update.
     * @param x X offset.
     * @param y Y offset.
     * @param width Width of the update region.
     * @param height Height of the update region.
     * @param pixels RGBA pixel data.
     */
    virtual void updateTexture(
        Texture& texture,
        u32 x, u32 y, u32 width, u32 height,
        const u8* pixels
    ) = 0;
    
    /**
     * @brief Destroy a texture.
     */
    virtual void destroyTexture(Texture& texture) = 0;
    
    /**
     * @brief Create a single-channel (alpha) texture for font atlases.
     * @param width Texture width.
     * @param height Texture height.
     * @param pixels Single-channel pixel data (1 byte per pixel).
     * @return The created texture, or nullptr on failure.
     * 
     * Note: The default implementation converts to RGBA and calls createTexture.
     * Override this for more efficient single-channel texture support.
     */
    virtual Ref<Texture> createAlphaTexture(u32 width, u32 height, const u8* pixels) {
        // Default: convert single-channel to RGBA
        std::vector<u8> rgba(width * height * 4);
        for (u32 i = 0; i < width * height; ++i) {
            rgba[i * 4 + 0] = 255;       // R
            rgba[i * 4 + 1] = 255;       // G
            rgba[i * 4 + 2] = 255;       // B
            rgba[i * 4 + 3] = pixels[i]; // A from source
        }
        return createTexture(width, height, rgba.data());
    }
    
    // -------------------------------------------------------------------------
    // Text rendering
    // -------------------------------------------------------------------------
    
    /**
     * @brief Measure text size.
     */
    virtual Size2f measureText(const TextLayout& layout) = 0;
    
    /**
     * @brief Render text to vertices.
     * This is called by RenderContext to generate vertices for text.
     */
    virtual void renderText(
        const TextLayout& layout,
        Point2f position,
        Color color,
        std::vector<Vertex>& vertices,
        std::vector<u32>& indices
    ) = 0;
    
    /**
     * @brief Get the font texture atlas (if applicable).
     */
    virtual Texture* fontAtlas() { return nullptr; }
    
    /**
     * @brief Set the backdrop source texture for frosted glass effects.
     * 
     * The host application should call this each frame with its rendered
     * scene framebuffer. Panels with backdropBlur > 0 will sample and
     * blur this texture to create a frosted glass effect.
     * 
     * @param texture The source framebuffer texture (read-only). Pass nullptr to disable.
     */
    virtual void setBackdropSource(Texture* texture) { m_backdropSource = texture; }
    
    /**
     * @brief Get the current backdrop source texture.
     */
    Texture* backdropSource() const { return m_backdropSource; }

protected:
    Texture* m_backdropSource{nullptr};
};

/**
 * @brief Null render backend for testing.
 */
class GUT_API NullRenderBackend : public RenderBackend {
public:
    bool initialize() override { return true; }
    void shutdown() override {}
    void beginFrame(u32, u32, f32) override {}
    void endFrame() override {}
    void render(std::span<const Vertex>, std::span<const u32>, std::span<const DrawCommand>) override {}
    Ref<Texture> createTexture(u32, u32, const u8*) override { return nullptr; }
    Ref<Texture> createTextureFromFile(StringView) override { return nullptr; }
    void updateTexture(Texture&, u32, u32, u32, u32, const u8*) override {}
    void destroyTexture(Texture&) override {}
    Size2f measureText(const TextLayout&) override { return {}; }
    void renderText(const TextLayout&, Point2f, Color, std::vector<Vertex>&, std::vector<u32>&) override {}
};

} // namespace gut


// --- gut/render/RenderContext.h ---


#include <vector>
#include <stack>

namespace gut {

// Forward declarations
class RenderBackend;
class Texture;
class Font;

/**
 * @brief Text layout information.
 */
struct GUT_API TextLayout {
    String text;
    String fontFamily;
    f32 fontSize{14.0f};
    bool bold{false};
    bool italic{false};
    f32 maxWidth{INFINITY};
    f32 lineHeight{1.2f};
    
    // Computed
    Size2f size{};
};

// Forward declarations
class FontFace;

/**
 * @brief Render context for drawing UI elements.
 * 
 * Provides an immediate-mode drawing API that is translated to
 * render backend calls. Supports transformations, clipping, and
 * drawing primitives.
 */
class GUT_API RenderContext {
public:
    explicit RenderContext(RenderBackend& backend);
    ~RenderContext();
    
    GUT_NONCOPYABLE(RenderContext)
    
    // -------------------------------------------------------------------------
    // Frame control
    // -------------------------------------------------------------------------
    
    /**
     * @brief Begin a new frame.
     */
    void beginFrame(Size2f size, f32 devicePixelRatio = 1.0f);
    
    /**
     * @brief End the frame and flush all drawing commands.
     */
    void endFrame();
    
    /**
     * @brief Get the frame size.
     */
    Size2f frameSize() const { return m_frameSize; }
    
    /**
     * @brief Get the device pixel ratio.
     */
    f32 devicePixelRatio() const { return m_devicePixelRatio; }
    
    // -------------------------------------------------------------------------
    // State management
    // -------------------------------------------------------------------------
    
    /**
     * @brief Save the current state (transform, clip, opacity).
     */
    void save();
    
    /**
     * @brief Restore the previously saved state.
     */
    void restore();
    
    // -------------------------------------------------------------------------
    // Transformations
    // -------------------------------------------------------------------------
    
    /**
     * @brief Translate the coordinate system.
     */
    void translate(f32 x, f32 y);
    void translate(Point2f offset) { translate(offset.x, offset.y); }
    
    /**
     * @brief Scale the coordinate system.
     */
    void scale(f32 sx, f32 sy);
    void scale(f32 s) { scale(s, s); }
    
    /**
     * @brief Rotate the coordinate system.
     * @param angle Rotation angle in radians.
     */
    void rotate(f32 angle);

    /**
     * @brief Skew the coordinate system.
     * @param skewX Horizontal skew angle in radians.
     * @param skewY Vertical skew angle in radians.
     */
    void skew(f32 skewX, f32 skewY);

    /**
     * @brief Transform a point from local to screen coordinates.
     */
    Point2f transformPoint(Point2f point) const;
    
    // -------------------------------------------------------------------------
    // Clipping
    // -------------------------------------------------------------------------
    
    /**
     * @brief Push a rectangular clip region.
     */
    void pushClip(Rectf rect, f32 cornerRadius = 0);
    
    /**
     * @brief Push a path-based clip region.
     */
    void pushClip(const Path& path);
    
    /**
     * @brief Pop the last clip region.
     */
    void popClip();
    
    // -------------------------------------------------------------------------
    // Opacity
    // -------------------------------------------------------------------------
    
    /**
     * @brief Set the global opacity (multiplied with element opacity).
     */
    void setOpacity(f32 opacity);
    
    /**
     * @brief Get the current global opacity.
     */
    f32 opacity() const;
    
    // -------------------------------------------------------------------------
    // Drawing primitives
    // -------------------------------------------------------------------------
    
    /**
     * @brief Fill a rectangle.
     */
    void fillRect(Rectf rect, Color color);
    void fillRect(Rectf rect, const Brush& brush);
    
    /**
     * @brief Stroke a rectangle.
     */
    void strokeRect(Rectf rect, Color color, f32 thickness = 1.0f);
    void strokeRect(Rectf rect, const Pen& pen);
    
    /**
     * @brief Fill a rectangle with a vertical gradient.
     */
    void fillRectGradient(Rectf rect, Color topColor, Color bottomColor);

    /**
     * @brief Fill a rounded rectangle.
     */
    void fillRoundedRect(Rectf rect, f32 cornerRadius, Color color);
    void fillRoundedRect(Rectf rect, f32 cornerRadius, const Brush& brush);
    void fillRoundedRect(Rectf rect, f32 topLeft, f32 topRight, f32 bottomRight, f32 bottomLeft, const Brush& brush);

    /**
     * @brief Fill a rounded rectangle with a vertical gradient.
     */
    void fillRoundedRectGradient(Rectf rect, f32 cornerRadius, Color topColor, Color bottomColor);
    
    /**
     * @brief Stroke a rounded rectangle.
     */
    void strokeRoundedRect(Rectf rect, f32 cornerRadius, Color color, f32 thickness = 1.0f);
    void strokeRoundedRect(Rectf rect, f32 cornerRadius, const Pen& pen);
    
    /**
     * @brief Draw a drop shadow around a rounded rect.
     * Builds a single quad-strip ring mesh: inner contour at shadow color,
     * outer contour at alpha=0. GPU interpolation creates a smooth fade.
     */
    void drawDropShadow(Rectf rect, f32 cornerRadius, Color shadowColor,
                        f32 blurRadius, f32 offsetX = 0, f32 offsetY = 4);
    
    /**
     * @brief Draw an inset (inner) shadow inside a rounded rect.
     *
     * Builds a quad-strip ring: outer contour at the rect boundary (full
     * shadow colour), inner contour inset by blurRadius (alpha = 0).
     * The result is a soft shadow that fades toward the centre.
     *
     * @param rect          The element rectangle.
     * @param cornerRadius  Corner rounding of the element.
     * @param shadowColor   Shadow colour (typically semi-transparent black).
     * @param blurRadius    How far the shadow fades inward.
     * @param offsetX       Horizontal offset (positive = shadow shifts right → left edge darker).
     * @param offsetY       Vertical offset (positive = shadow shifts down → top edge darker).
     */
    void drawInsetShadow(Rectf rect, f32 cornerRadius, Color shadowColor,
                         f32 blurRadius, f32 offsetX = 0, f32 offsetY = 0);
    
    /**
     * @brief Fill an ellipse.
     */
    void fillEllipse(Point2f center, f32 radiusX, f32 radiusY, Color color);
    void fillEllipse(Point2f center, f32 radiusX, f32 radiusY, const Brush& brush);
    
    /**
     * @brief Stroke an ellipse.
     */
    void strokeEllipse(Point2f center, f32 radiusX, f32 radiusY, Color color, f32 thickness = 1.0f);
    void strokeEllipse(Point2f center, f32 radiusX, f32 radiusY, const Pen& pen);
    
    /**
     * @brief Fill a circle.
     */
    void fillCircle(Point2f center, f32 radius, Color color);
    void fillCircle(Point2f center, f32 radius, const Brush& brush);
    
    /**
     * @brief Stroke a circle.
     */
    void strokeCircle(Point2f center, f32 radius, Color color, f32 thickness = 1.0f);
    void strokeCircle(Point2f center, f32 radius, const Pen& pen);
    
    /**
     * @brief Draw a line.
     */
    void drawLine(Point2f p1, Point2f p2, Color color, f32 thickness = 1.0f);
    void drawLine(Point2f p1, Point2f p2, const Pen& pen);
    
    /**
     * @brief Draw a polyline (connected line segments).
     *
     * Proper miter/bevel/round joins and flat/square/round end-caps.
     */
    void drawPolyline(const Point2f* points, u32 count, Color color, f32 thickness = 1.0f,
                      bool closed = false, LineJoin join = LineJoin::Miter,
                      LineCap cap = LineCap::Flat, f32 miterLimit = 10.0f);
    void drawPolyline(const std::vector<Point2f>& pts, Color color, f32 thickness = 1.0f,
                      bool closed = false, LineJoin join = LineJoin::Miter,
                      LineCap cap = LineCap::Flat, f32 miterLimit = 10.0f);
    void drawPolyline(const Point2f* points, u32 count, const Pen& pen, bool closed = false);

    /**
     * @brief Fill a polygon (triangle-fan ear clipping).
     */
    void fillPolygon(const Point2f* points, u32 count, Color color);
    void fillPolygon(const std::vector<Point2f>& pts, Color color);

    /**
     * @brief Fill a triangle.
     */
    void fillTriangle(Point2f p1, Point2f p2, Point2f p3, Color color);

    /**
     * @brief Fill a path.
     *
     * Flattens curves to polylines, then triangulates each sub-path
     * via ear-clipping.
     */
    void fillPath(const Path& path, Color color);
    void fillPath(const Path& path, const Brush& brush);
    
    /**
     * @brief Stroke a path.
     *
     * Flattens curves to polylines, then strokes each sub-path with
     * proper thickness, joins, and end-caps.
     */
    void strokePath(const Path& path, Color color, f32 thickness = 1.0f);
    void strokePath(const Path& path, const Pen& pen);
    
    // -------------------------------------------------------------------------
    // Images
    // -------------------------------------------------------------------------
    
    /**
     * @brief Draw an image.
     */
    void drawImage(const Texture& texture, Rectf destRect);
    void drawImage(const Texture& texture, Rectf destRect, Color tint);
    void drawImage(const Texture& texture, Rectf srcRect, Rectf destRect);
    void drawImage(const Texture& texture, Rectf srcRect, Rectf destRect, Color tint);
    
    /**
     * @brief Draw an image using 9-slice scaling.
     *
     * The source image is divided into a 3×3 grid by the border insets.
     * Corners stay fixed-size, edges stretch in one axis, center stretches
     * in both axes.  This preserves styled borders at any destination size.
     *
     * @param texture   The source texture.
     * @param borders   Pixel insets {left, top, right, bottom} defining the 9 slices.
     * @param destRect  Destination rectangle in local coordinates.
     * @param tint      Color multiplier (default white = no tint).
     */
    void drawImageNineSlice(const Texture& texture, Thickness borders,
                            Rectf destRect, Color tint = Color::white());
    void drawImageNineSlice(const Texture& texture, Rectf srcRect,
                            Thickness borders, Rectf destRect,
                            Color tint = Color::white());
    
    // -------------------------------------------------------------------------
    // Text
    // -------------------------------------------------------------------------
    
    /**
     * @brief Measure text size.
     */
    Size2f measureText(const TextLayout& layout);
    
    /**
     * @brief Draw text.
     */
    void drawText(const TextLayout& layout, Point2f position, Color color);
    
    /**
     * @brief Draw text with a font face directly.
     * @param face The font face to use.
     * @param text The text string to draw.
     * @param position The position (left, baseline).
     * @param color The text color.
     * 
     * This is a convenience method that bypasses TextLayout for simple text.
     */
    void drawText(FontFace* face, const std::string& text, Point2f position, Color color);

    /**
     * @brief Draw text with word wrapping and alignment.
     * @param face The font face to use.
     * @param text The text string to draw.
     * @param position The position (left, baseline of first line).
     * @param color The text color.
     * @param maxWidth The maximum width for word wrapping (0 = no wrap).
     * @param alignment Text alignment within maxWidth.
     */
    void drawTextMultiline(FontFace* face, const std::string& text, Point2f position, Color color,
                           f32 maxWidth, TextAlignment alignment = TextAlignment::Left);
    
    /**
     * @brief Draw a backdrop blur effect (frosted glass).
     * @param rect The rectangle to blur in local coordinates.
     * @param cornerRadius Corner radius for the blurred region.
     * @param blurRadius Blur strength in logical pixels.
     * @param tint Tint color applied over the blurred backdrop.
     */
    void drawBackdropBlur(Rectf rect, f32 cornerRadius, f32 blurRadius, Color tint);
    
    // -------------------------------------------------------------------------
    // Backend access
    // -------------------------------------------------------------------------
    
    /**
     * @brief Get the render backend.
     */
    RenderBackend& backend() { return m_backend; }
    
    /**
     * @brief Flush pending draw commands to the backend.
     */
    void flush();

private:
    struct State {
        // 2D affine transform matrix stored as [a b tx; c d ty; 0 0 1]
        // where transformPoint(x,y) = (a*x + b*y + tx, c*x + d*y + ty)
        f32 m[6]{1, 0, 0, 0, 1, 0}; // identity: a=1 b=0 tx=0 c=0 d=1 ty=0
        f32 opacity{1};
        Rectf clipRect{};
        f32 clipCornerRadius{0};
        bool hasClip{false};
    };
    
    // Helper to add vertices and get back the starting index
    u32 addVertices(const Vertex* verts, u32 count);
    u32 addIndices(const u32* inds, u32 count, u32 vertexOffset);
    void addDrawCommand(DrawCommandType type, u32 indexCount, Texture* texture = nullptr);
    
    // Helper for common shapes
    void addRect(Rectf rect, Color color, Texture* texture = nullptr, Rectf uvRect = {0, 0, 1, 1});
    void addRectGradient(Rectf rect, Color topColor, Color bottomColor);
    void addRectRadialGradient(Rectf rect, const RadialGradientBrush& brush);
    void addRoundedRect(Rectf rect, f32 radius, Color color);
    void addRoundedRectGradient(Rectf rect, f32 radius, Color topColor, Color bottomColor);
    void addRoundedRectRadialGradient(Rectf rect, f32 radius, const RadialGradientBrush& brush);
    void addEllipseRadialGradient(Point2f center, f32 rx, f32 ry, const RadialGradientBrush& brush, i32 segments = 32);
    void addDropShadow(Rectf rect, f32 cornerRadius, Color shadowColor,
                       f32 blurRadius, f32 offsetX, f32 offsetY);
    void addInsetShadow(Rectf rect, f32 cornerRadius, Color shadowColor,
                       f32 blurRadius, f32 offsetX, f32 offsetY);
    void addEllipse(Point2f center, f32 rx, f32 ry, Color color, i32 segments = 32);
    void addLine(Point2f p1, Point2f p2, Color color, f32 thickness);
    void addPolyline(const Point2f* points, u32 count, Color color, f32 thickness,
                     bool closed, LineJoin join, LineCap cap, f32 miterLimit);
    void addFilledPolygon(const Point2f* points, u32 count, Color color);

    // Sample a color along a gradient given a normalized t (0=top, 1=bottom)
    Color sampleGradient(Color top, Color bottom, f32 t) {
        return Color(
            top.r + (bottom.r - top.r) * t,
            top.g + (bottom.g - top.g) * t,
            top.b + (bottom.b - top.b) * t,
            top.a + (bottom.a - top.a) * t
        );
    }

    /// Sample a multi-stop gradient at normalized parameter t ∈ [0,1].
    Color sampleGradientStops(const std::vector<GradientStop>& stops, f32 t) {
        if (stops.empty()) return Color(0, 0, 0, 0);
        if (stops.size() == 1 || t <= stops.front().offset) return stops.front().color;
        if (t >= stops.back().offset) return stops.back().color;
        for (usize i = 0; i + 1 < stops.size(); ++i) {
            if (t >= stops[i].offset && t <= stops[i + 1].offset) {
                f32 range = stops[i + 1].offset - stops[i].offset;
                f32 local = (range > 0.0f) ? (t - stops[i].offset) / range : 0.0f;
                return sampleGradient(stops[i].color, stops[i + 1].color, local);
            }
        }
        return stops.back().color;
    }
    
    RenderBackend& m_backend;
    Size2f m_frameSize{};
    f32 m_devicePixelRatio{1.0f};
    
    std::stack<State> m_stateStack;
    State m_currentState;
    
    // Draw command buffers
    std::vector<Vertex> m_vertices;
    std::vector<u32> m_indices;
    std::vector<DrawCommand> m_commands;
};

} // namespace gut


// --- gut/input/MouseEvent.h ---



namespace gut {

/**
 * @brief Mouse button identifiers.
 */
enum class MouseButton : u8 {
    None = 0,
    Left = 1,
    Right = 2,
    Middle = 3,
    X1 = 4,
    X2 = 5
};

/**
 * @brief Modifier key flags.
 */
enum class ModifierKeys : u8 {
    None    = 0,
    Shift   = 1 << 0,
    Control = 1 << 1,
    Alt     = 1 << 2,
    Super   = 1 << 3  // Windows key / Command key
};

inline ModifierKeys operator|(ModifierKeys a, ModifierKeys b) {
    return static_cast<ModifierKeys>(static_cast<u8>(a) | static_cast<u8>(b));
}

inline ModifierKeys operator&(ModifierKeys a, ModifierKeys b) {
    return static_cast<ModifierKeys>(static_cast<u8>(a) & static_cast<u8>(b));
}

inline bool hasModifier(ModifierKeys mods, ModifierKeys flag) {
    return (mods & flag) == flag;
}

/**
 * @brief Mouse event types.
 */
enum class MouseEventType : u8 {
    Move,
    ButtonDown,
    ButtonUp,
    DoubleClick,
    Wheel,
    Enter,
    Leave
};

/**
 * @brief Mouse event data.
 */
struct GUT_API MouseEvent {
    MouseEventType type{MouseEventType::Move};
    Point2f position{};         // Position in element coordinates
    Point2f screenPosition{};   // Position in screen coordinates
    Point2f delta{};            // Movement delta (for Move) or wheel delta (for Wheel)
    MouseButton button{MouseButton::None};
    ModifierKeys modifiers{ModifierKeys::None};
    u32 clickCount{0};          // Number of clicks (for double-click detection)
    bool handled{false};        // Set to true to stop event from bubbling further
    Element* source{nullptr};   // Original element that received the event (hit-test target)
    
    bool isLeftButton() const { return button == MouseButton::Left; }
    bool isRightButton() const { return button == MouseButton::Right; }
    bool isMiddleButton() const { return button == MouseButton::Middle; }
    
    bool hasShift() const { return hasModifier(modifiers, ModifierKeys::Shift); }
    bool hasControl() const { return hasModifier(modifiers, ModifierKeys::Control); }
    bool hasAlt() const { return hasModifier(modifiers, ModifierKeys::Alt); }
};

} // namespace gut


// --- gut/input/KeyEvent.h ---



namespace gut {

/**
 * @brief Key codes (based on USB HID usage codes, compatible with most platforms).
 */
enum class Key : u16 {
    Unknown = 0,
    
    // Letters
    A = 4, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    
    // Numbers
    Num1 = 30, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0,
    
    // Function keys
    Return = 40,
    Escape = 41,
    Backspace = 42,
    Tab = 43,
    Space = 44,
    
    // Symbols
    Minus = 45,
    Equals = 46,
    LeftBracket = 47,
    RightBracket = 48,
    Backslash = 49,
    Semicolon = 51,
    Apostrophe = 52,
    Grave = 53,
    Comma = 54,
    Period = 55,
    Slash = 56,
    
    CapsLock = 57,
    
    // Function keys
    F1 = 58, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    
    PrintScreen = 70,
    ScrollLock = 71,
    Pause = 72,
    Insert = 73,
    Home = 74,
    PageUp = 75,
    Delete = 76,
    End = 77,
    PageDown = 78,
    
    // Arrow keys
    Right = 79,
    Left = 80,
    Down = 81,
    Up = 82,
    
    NumLock = 83,
    
    // Numpad
    NumpadDivide = 84,
    NumpadMultiply = 85,
    NumpadMinus = 86,
    NumpadPlus = 87,
    NumpadEnter = 88,
    Numpad1 = 89, Numpad2, Numpad3, Numpad4, Numpad5,
    Numpad6, Numpad7, Numpad8, Numpad9, Numpad0,
    NumpadDecimal = 99,
    
    // Modifiers
    LeftControl = 224,
    LeftShift = 225,
    LeftAlt = 226,
    LeftSuper = 227,
    RightControl = 228,
    RightShift = 229,
    RightAlt = 230,
    RightSuper = 231
};

/**
 * @brief Key event types.
 */
enum class KeyEventType : u8 {
    KeyDown,
    KeyUp,
    Char  // Character input (text input)
};

/**
 * @brief Keyboard event data.
 */
struct GUT_API KeyEvent {
    KeyEventType type{KeyEventType::KeyDown};
    Key key{Key::Unknown};
    u32 scancode{0};            // Platform-specific scancode
    ModifierKeys modifiers{ModifierKeys::None};
    char32_t character{0};      // Unicode character (for Char events)
    bool repeat{false};         // Key repeat
    bool handled{false};        // Set to true to stop event from bubbling further
    Element* source{nullptr};   // Original element that received the event
    
    bool isKeyDown() const { return type == KeyEventType::KeyDown; }
    bool isKeyUp() const { return type == KeyEventType::KeyUp; }
    bool isChar() const { return type == KeyEventType::Char; }
    
    bool hasShift() const { return hasModifier(modifiers, ModifierKeys::Shift); }
    bool hasControl() const { return hasModifier(modifiers, ModifierKeys::Control); }
    bool hasAlt() const { return hasModifier(modifiers, ModifierKeys::Alt); }
};

} // namespace gut


// --- gut/input/FocusManager.h ---



namespace gut {

// Forward declarations
class Element;
class Context;

/**
 * @brief Focus navigation direction.
 */
enum class FocusDirection {
    Next,       // Tab
    Previous,   // Shift+Tab
    Up,
    Down,
    Left,
    Right
};

/**
 * @brief Manages keyboard focus for a gut Context.
 */
class GUT_API FocusManager {
public:
    explicit FocusManager(Context& context);
    ~FocusManager();
    
    GUT_NONCOPYABLE(FocusManager)
    
    // -------------------------------------------------------------------------
    // Focus management
    // -------------------------------------------------------------------------
    
    /**
     * @brief Get the currently focused element.
     */
    Element* focusedElement() const { return m_focusedElement; }
    
    /**
     * @brief Set focus to an element.
     * @param element The element to focus, or nullptr to clear focus.
     */
    void setFocus(Element* element);
    
    /**
     * @brief Clear focus (no element focused).
     */
    void clearFocus();
    
    /**
     * @brief Move focus in a direction.
     */
    void moveFocus(FocusDirection direction);
    
    /**
     * @brief Focus the next focusable element.
     */
    void focusNext();
    
    /**
     * @brief Focus the previous focusable element.
     */
    void focusPrevious();
    
    // -------------------------------------------------------------------------
    // Signals
    // -------------------------------------------------------------------------
    
    /**
     * @brief Emitted when focus changes.
     * @param oldFocus The previously focused element (may be nullptr).
     * @param newFocus The newly focused element (may be nullptr).
     */
    Signal<Element*, Element*>& focusChanged() { return m_focusChanged; }

private:
    Element* findNextFocusable(Element* current, bool forward);
    Element* findFocusableInDirection(Element* current, FocusDirection direction);
    void collectFocusableElements(Element* root, std::vector<Element*>& result);
    
    Context& m_context;
    Element* m_focusedElement{nullptr};
    Signal<Element*, Element*> m_focusChanged;
};

} // namespace gut


// --- gut/input/InputManager.h ---



namespace gut {

// Forward declarations
class Element;
class Context;

/**
 * @brief Manages input routing and state for a gut Context.
 */
class GUT_API InputManager {
public:
    explicit InputManager(Context& context);
    ~InputManager();
    
    GUT_NONCOPYABLE(InputManager)
    
    // -------------------------------------------------------------------------
    // Input injection (call these from your game loop)
    // -------------------------------------------------------------------------
    
    /**
     * @brief Process a mouse move event.
     */
    void processMouseMove(f32 x, f32 y);
    
    /**
     * @brief Process a mouse button event.
     */
    void processMouseButton(MouseButton button, bool pressed);
    
    /**
     * @brief Process a mouse wheel event.
     */
    void processMouseWheel(f32 deltaX, f32 deltaY);
    
    /**
     * @brief Process a key event.
     */
    void processKey(Key key, bool pressed, ModifierKeys modifiers = ModifierKeys::None);
    
    /**
     * @brief Process a text input event.
     */
    void processTextInput(char32_t character);
    
    // -------------------------------------------------------------------------
    // State queries
    // -------------------------------------------------------------------------
    
    /**
     * @brief Get the current mouse position.
     */
    Point2f mousePosition() const { return m_mousePosition; }
    
    /**
     * @brief Check if a mouse button is currently pressed.
     */
    bool isMouseButtonDown(MouseButton button) const;
    
    /**
     * @brief Check if a key is currently pressed.
     */
    bool isKeyDown(Key key) const;
    
    /**
     * @brief Get current modifier keys state.
     */
    ModifierKeys modifiers() const { return m_modifiers; }
    
    /**
     * @brief Get the element currently under the mouse.
     */
    Element* hoveredElement() const { return m_hoveredElement; }
    
    /**
     * @brief Get the element that has captured mouse input.
     */
    Element* capturedElement() const { return m_capturedElement; }
    
    // -------------------------------------------------------------------------
    // Mouse capture
    // -------------------------------------------------------------------------
    
    /**
     * @brief Capture mouse input to an element.
     */
    void captureMouse(Element* element);
    
    /**
     * @brief Release mouse capture.
     */
    void releaseMouse();
    
    // -------------------------------------------------------------------------
    // Double-click handling
    // -------------------------------------------------------------------------
    
    /**
     * @brief Set the double-click interval in milliseconds.
     */
    void setDoubleClickInterval(u32 ms) { m_doubleClickInterval = ms; }
    
    /**
     * @brief Set the maximum distance for double-click.
     */
    void setDoubleClickDistance(f32 distance) { m_doubleClickDistance = distance; }

private:
    void updateHoveredElement();
    MouseEvent createMouseEvent(MouseEventType type, MouseButton button = MouseButton::None);
    KeyEvent createKeyEvent(KeyEventType type, Key key);
    
    /// Dispatch a mouse event to target, bubbling up through parent() chain.
    /// Enter/Leave events do NOT bubble (per WPF convention).
    void dispatchMouseEvent(Element* target, MouseEvent event);
    
    /// Dispatch a key event to target, bubbling up through parent() chain.
    void dispatchKeyEvent(Element* target, KeyEvent event);
    
    Context& m_context;
    
    Point2f m_mousePosition{};
    u32 m_mouseButtonState{0};
    u64 m_keyState[4]{0};  // Bit field for 256 keys
    ModifierKeys m_modifiers{ModifierKeys::None};
    
    Element* m_hoveredElement{nullptr};
    Element* m_capturedElement{nullptr};
    Element* m_pressedElement{nullptr};
    
    // Double-click tracking
    u32 m_doubleClickInterval{400};  // ms
    f32 m_doubleClickDistance{5.0f};
    u64 m_lastClickTime{0};
    Point2f m_lastClickPosition{};
    MouseButton m_lastClickButton{MouseButton::None};
    u32 m_clickCount{0};
};

} // namespace gut


// --- gut/animation/Easing.h ---


#include <functional>
#include <string>
#include <unordered_map>

namespace gut {

/// Easing function type: takes normalized time (0-1), returns eased value (usually 0-1)
using EasingFunction = std::function<f32(f32)>;

/// Standard easing functions
namespace easing {

// Linear
GUT_API f32 linear(f32 t);

// Quadratic
GUT_API f32 easeInQuad(f32 t);
GUT_API f32 easeOutQuad(f32 t);
GUT_API f32 easeInOutQuad(f32 t);

// Cubic
GUT_API f32 easeInCubic(f32 t);
GUT_API f32 easeOutCubic(f32 t);
GUT_API f32 easeInOutCubic(f32 t);

// Quartic
GUT_API f32 easeInQuart(f32 t);
GUT_API f32 easeOutQuart(f32 t);
GUT_API f32 easeInOutQuart(f32 t);

// Quintic
GUT_API f32 easeInQuint(f32 t);
GUT_API f32 easeOutQuint(f32 t);
GUT_API f32 easeInOutQuint(f32 t);

// Sine
GUT_API f32 easeInSine(f32 t);
GUT_API f32 easeOutSine(f32 t);
GUT_API f32 easeInOutSine(f32 t);

// Exponential
GUT_API f32 easeInExpo(f32 t);
GUT_API f32 easeOutExpo(f32 t);
GUT_API f32 easeInOutExpo(f32 t);

// Circular
GUT_API f32 easeInCirc(f32 t);
GUT_API f32 easeOutCirc(f32 t);
GUT_API f32 easeInOutCirc(f32 t);

// Back (overshoot)
GUT_API f32 easeInBack(f32 t);
GUT_API f32 easeOutBack(f32 t);
GUT_API f32 easeInOutBack(f32 t);

// Elastic
GUT_API f32 easeInElastic(f32 t);
GUT_API f32 easeOutElastic(f32 t);
GUT_API f32 easeInOutElastic(f32 t);

// Bounce
GUT_API f32 easeInBounce(f32 t);
GUT_API f32 easeOutBounce(f32 t);
GUT_API f32 easeInOutBounce(f32 t);

} // namespace easing

/// Registry of easing functions by name
class GUT_API EasingRegistry {
public:
    static EasingRegistry& instance();
    
    /// Register a custom easing function
    void registerFunction(const std::string& name, EasingFunction func);
    
    /// Get easing function by name, returns linear if not found
    EasingFunction get(const std::string& name) const;
    
    /// Check if function exists
    bool has(const std::string& name) const;
    
private:
    EasingRegistry();
    std::unordered_map<std::string, EasingFunction> m_functions;
};

/// Cubic bezier easing (CSS-style)
class GUT_API CubicBezierEasing {
public:
    CubicBezierEasing(f32 x1, f32 y1, f32 x2, f32 y2);
    
    f32 operator()(f32 t) const;
    
    // Common presets
    static CubicBezierEasing ease();        // (0.25, 0.1, 0.25, 1.0)
    static CubicBezierEasing easeIn();      // (0.42, 0.0, 1.0, 1.0)
    static CubicBezierEasing easeOut();     // (0.0, 0.0, 0.58, 1.0)
    static CubicBezierEasing easeInOut();   // (0.42, 0.0, 0.58, 1.0)
    
private:
    f32 sampleCurveX(f32 t) const;
    f32 sampleCurveY(f32 t) const;
    f32 sampleCurveDerivativeX(f32 t) const;
    f32 solveCurveX(f32 x, f32 epsilon) const;
    
    f32 m_x1, m_y1, m_x2, m_y2;
};

/// Spring-based easing for physics-like animations
class GUT_API SpringEasing {
public:
    SpringEasing(f32 mass = 1.0f, f32 stiffness = 100.0f, f32 damping = 10.0f);
    
    f32 operator()(f32 t) const;
    
    void setMass(f32 mass) { m_mass = mass; }
    void setStiffness(f32 stiffness) { m_stiffness = stiffness; }
    void setDamping(f32 damping) { m_damping = damping; }
    
private:
    f32 m_mass;
    f32 m_stiffness;
    f32 m_damping;
};

} // namespace gut


// --- gut/animation/Animation.h ---


#include <functional>
#include <string>

namespace gut {

class Element;
class Timeline;

/// Animation state
enum class AnimationState {
    Stopped,
    Playing,
    Paused
};

/// Fill behavior after animation completes
enum class FillBehavior {
    HoldEnd,    // Hold the final value
    Stop        // Return to original value
};

/// Animation repeat behavior
struct RepeatBehavior {
    enum class Type { Count, Forever, Duration };
    Type type = Type::Count;
    f32 count = 1.0f;
    f32 durationMs = 0.0f;
    
    static RepeatBehavior once() { return {Type::Count, 1.0f, 0.0f}; }
    static RepeatBehavior forever() { return {Type::Forever, 0.0f, 0.0f}; }
    static RepeatBehavior times(f32 n) { return {Type::Count, n, 0.0f}; }
    static RepeatBehavior duration(f32 ms) { return {Type::Duration, 0.0f, ms}; }
};

/// Base class for all animations
class GUT_API Animation : public Object {
    GUT_OBJECT(Animation, Object)
    
public:
    Animation();
    virtual ~Animation();
    
    // Timing properties
    GUT_PROPERTY(f32, duration, 1000.0f)          // Duration in milliseconds
    GUT_PROPERTY(f32, beginTime, 0.0f)            // Delay before starting
    GUT_PROPERTY(f32, speedRatio, 1.0f)           // Speed multiplier
    GUT_PROPERTY(bool, autoReverse, false)        // Reverse after completing
    GUT_PROPERTY(FillBehavior, fillBehavior, FillBehavior::HoldEnd)
    
    RepeatBehavior repeatBehavior() const { return m_repeatBehavior; }
    void setRepeatBehavior(RepeatBehavior behavior) { m_repeatBehavior = behavior; }
    
    void setEasingFunction(EasingFunction fn) { m_easingFunction = std::move(fn); }
    const EasingFunction& easingFunction() const { return m_easingFunction; }
    
    // State
    AnimationState state() const { return m_state; }
    f32 currentTime() const { return m_currentTime; }
    f32 progress() const; // 0.0 to 1.0
    
    // Control
    void begin();
    void pause();
    void resume();
    void stop();
    void seek(f32 timeMs);
    
    // Signals
    Signal<> completed;
    Signal<> started;
    
    // Called by Timeline
    virtual void update(f32 deltaMs);
    
protected:
    // Override to apply animation value at given progress (0-1)
    virtual void applyValue(f32 normalizedTime) = 0;
    
    AnimationState m_state = AnimationState::Stopped;
    RepeatBehavior m_repeatBehavior;
    EasingFunction m_easingFunction;
    f32 m_currentTime = 0.0f;
    f32 m_totalElapsed = 0.0f;
    i32 m_currentIteration = 0;
    bool m_isReversing = false;
};

/// Animation that interpolates a property value
template<typename T>
class PropertyAnimation : public Animation {
    GUT_OBJECT(PropertyAnimation, Animation)
    
public:
    PropertyAnimation() = default;
    
    void setTargetProperty(Property<T>* prop) { m_targetProperty = prop; }
    Property<T>* targetProperty() const { return m_targetProperty; }
    
    void setFrom(T value) { m_from = value; m_hasFrom = true; }
    void setTo(T value) { m_to = value; m_hasTo = true; }
    void setBy(T value) { m_by = value; m_hasBy = true; }
    
    T from() const { return m_from; }
    T to() const { return m_to; }
    T by() const { return m_by; }
    
protected:
    void applyValue(f32 t) override {
        if (!m_targetProperty) return;
        
        T startValue = m_hasFrom ? m_from : m_targetProperty->get();
        T endValue;
        
        if (m_hasTo) {
            endValue = m_to;
        } else if (m_hasBy) {
            endValue = startValue + m_by;
        } else {
            return;
        }
        
        // Linear interpolation
        T interpolated = lerp(startValue, endValue, t);
        m_targetProperty->set(interpolated);
    }
    
private:
    static T lerp(const T& a, const T& b, f32 t) {
        return a + (b - a) * t;
    }
    
    Property<T>* m_targetProperty = nullptr;
    T m_from{};
    T m_to{};
    T m_by{};
    bool m_hasFrom = false;
    bool m_hasTo = false;
    bool m_hasBy = false;
};

// Common animation type aliases
using FloatAnimation = PropertyAnimation<f32>;
using DoubleAnimation = PropertyAnimation<f64>;
using ColorAnimation = PropertyAnimation<Color>;
using PointAnimation = PropertyAnimation<Point2f>;
using SizeAnimation = PropertyAnimation<Size2f>;
using RectAnimation = PropertyAnimation<Rectf>;

/// Keyframe for keyframe animations
template<typename T>
struct KeyFrame {
    f32 keyTime;  // Time as percentage (0-1) of total duration
    T value;
    std::string easingFunction;  // Name of easing function
};

/// Animation using keyframes
template<typename T>
class KeyFrameAnimation : public Animation {
    GUT_OBJECT(KeyFrameAnimation, Animation)
    
public:
    void setTargetProperty(Property<T>* prop) { m_targetProperty = prop; }
    
    void addKeyFrame(f32 keyTime, T value, const std::string& easing = "linear") {
        m_keyFrames.push_back({keyTime, value, easing});
        // Keep sorted by keyTime
        std::sort(m_keyFrames.begin(), m_keyFrames.end(),
                  [](const auto& a, const auto& b) { return a.keyTime < b.keyTime; });
    }
    
    void clearKeyFrames() { m_keyFrames.clear(); }
    
protected:
    void applyValue(f32 t) override {
        if (!m_targetProperty || m_keyFrames.empty()) return;
        
        // Find surrounding keyframes
        const KeyFrame<T>* before = nullptr;
        const KeyFrame<T>* after = nullptr;
        
        for (const auto& kf : m_keyFrames) {
            if (kf.keyTime <= t) {
                before = &kf;
            }
            if (kf.keyTime >= t && !after) {
                after = &kf;
            }
        }
        
        if (!before && !after) return;
        if (!before) {
            m_targetProperty->setValue(after->value);
            return;
        }
        if (!after || before == after) {
            m_targetProperty->setValue(before->value);
            return;
        }
        
        // Interpolate between keyframes
        f32 segmentProgress = (t - before->keyTime) / (after->keyTime - before->keyTime);
        T interpolated = lerp(before->value, after->value, segmentProgress);
        m_targetProperty->setValue(interpolated);
    }
    
private:
    static T lerp(const T& a, const T& b, f32 t) {
        return a + (b - a) * t;
    }
    
    Property<T>* m_targetProperty = nullptr;
    std::vector<KeyFrame<T>> m_keyFrames;
};

/// Storyboard groups multiple animations together
class GUT_API Storyboard : public Object {
    GUT_OBJECT(Storyboard, Object)
    
public:
    Storyboard();
    ~Storyboard();
    
    void addAnimation(Ref<Animation> animation);
    void removeAnimation(Animation* animation);
    void clear();
    
    const std::vector<Ref<Animation>>& animations() const { return m_animations; }
    
    // Control all animations
    void begin();
    void pause();
    void resume();
    void stop();
    
    // Update all animations
    void update(f32 deltaMs);
    
    Signal<> completed;  // All animations completed
    
private:
    std::vector<Ref<Animation>> m_animations;
    i32 m_completedCount = 0;
};

} // namespace gut


// --- gut/animation/Timeline.h ---


#include <vector>
#include <memory>

namespace gut {

class Animation;
class Storyboard;

/// Global animation timeline that ticks all registered animations
class GUT_API Timeline : public Object {
    GUT_OBJECT(Timeline, Object)
    
public:
    Timeline();
    ~Timeline();
    
    /// Get the global timeline instance
    static Timeline* global();
    
    /// Register an animation to be ticked
    void add(Animation* animation);
    
    /// Remove an animation from ticking
    void remove(Animation* animation);
    
    /// Register a storyboard to be ticked
    void add(Storyboard* storyboard);
    
    /// Remove a storyboard from ticking
    void remove(Storyboard* storyboard);
    
    /// Tick all animations by deltaMs milliseconds
    void update(f32 deltaMs);
    
    /// Pause all animations
    void pauseAll();
    
    /// Resume all paused animations
    void resumeAll();
    
    /// Stop all animations
    void stopAll();
    
    /// Current timeline time in milliseconds
    f32 currentTime() const { return m_currentTime; }
    
    /// Check if any animations are running
    bool hasActiveAnimations() const;
    
    /// Get/set global time scale
    f32 timeScale() const { return m_timeScale; }
    void setTimeScale(f32 scale) { m_timeScale = scale; }
    
private:
    std::vector<Animation*> m_animations;
    std::vector<Storyboard*> m_storyboards;
    f32 m_currentTime = 0.0f;
    f32 m_timeScale = 1.0f;
    bool m_updating = false;
    std::vector<Animation*> m_pendingRemoveAnimations;
    std::vector<Storyboard*> m_pendingRemoveStoryboards;
};

} // namespace gut


// --- gut/style/Style.h ---


#include <string>
#include <vector>
#include <variant>
#include <optional>

namespace gut {

// Forward declarations
class Element;
class StyleSheet;

/**
 * @brief CSS-like selector for matching elements.
 */
class GUT_API Selector {
public:
    /**
     * @brief Selector part types.
     */
    enum class PartType {
        Type,           // Element type name (e.g., "Button")
        Id,             // ID selector (e.g., "#myButton")
        Class,          // Class selector (e.g., ".primary")
        PseudoClass,    // Pseudo-class (e.g., ":hover", ":focused")
        Universal       // Universal selector (*)
    };
    
    /**
     * @brief Combinator between selector parts.
     */
    enum class Combinator {
        None,           // Direct match (no combinator)
        Descendant,     // Space - any descendant
        Child,          // > - direct child
        Adjacent,       // + - adjacent sibling
        Sibling         // ~ - general sibling
    };
    
    /**
     * @brief Single selector part.
     */
    struct Part {
        PartType type;
        String value;
        
        Part(PartType t, String v) : type(t), value(std::move(v)) {}
    };
    
    /**
     * @brief Selector segment (one or more parts with a combinator).
     */
    struct Segment {
        std::vector<Part> parts;  // Multiple parts can apply (e.g., "Button.primary:hover")
        Combinator combinator{Combinator::None};
    };
    
    Selector() = default;
    
    /**
     * @brief Parse a selector string.
     */
    static Selector parse(StringView selectorStr);
    
    /**
     * @brief Check if this selector matches an element.
     */
    bool matches(const Element& element) const;
    
    /**
     * @brief Get the specificity of this selector.
     * Returns a tuple of (ids, classes, types).
     */
    std::tuple<u32, u32, u32> specificity() const;
    
    /**
     * @brief Compare specificity with another selector.
     * @return true if this selector is more specific.
     */
    bool moreSpecificThan(const Selector& other) const;
    
    /**
     * @brief Get the selector as a string.
     */
    String toString() const;
    
    /**
     * @brief Check if selector is valid.
     */
    bool isValid() const { return !m_segments.empty(); }
    
    const std::vector<Segment>& segments() const { return m_segments; }

private:
    bool matchesSegment(const Element& element, usize segmentIndex) const;
    bool matchesPart(const Element& element, const Part& part) const;
    
    std::vector<Segment> m_segments;
};

/**
 * @brief Style property value.
 */
using StyleValue = std::variant<
    std::monostate,     // Unset
    f32,                // Number
    i32,                // Integer  
    String,             // String
    Color,              // Color
    Thickness,          // Thickness (margins, padding, etc.)
    HorizontalAlignment,
    VerticalAlignment,
    Visibility
>;

/**
 * @brief A style rule: selector + property declarations.
 */
class GUT_API StyleRule {
public:
    StyleRule() = default;
    StyleRule(Selector selector);
    
    /**
     * @brief Set a property value.
     */
    void setProperty(StringView name, StyleValue value);
    
    /**
     * @brief Get a property value.
     */
    std::optional<StyleValue> getProperty(StringView name) const;
    
    /**
     * @brief Check if this rule has a property.
     */
    bool hasProperty(StringView name) const;
    
    /**
     * @brief Get the selector.
     */
    const Selector& selector() const { return m_selector; }
    
    /**
     * @brief Get all properties.
     */
    const std::unordered_map<String, StyleValue>& properties() const { return m_properties; }

private:
    Selector m_selector;
    std::unordered_map<String, StyleValue> m_properties;
};

/**
 * @brief Inline style that can be applied directly to an element.
 */
class GUT_API Style : public Object {
    GUT_OBJECT(Style, Object)
    
public:
    Style() = default;
    ~Style() override = default;
    
    /**
     * @brief Set a property value.
     */
    void set(StringView name, StyleValue value);
    
    /**
     * @brief Get a property value.
     */
    std::optional<StyleValue> get(StringView name) const;
    
    /**
     * @brief Check if a property is set.
     */
    bool has(StringView name) const;
    
    /**
     * @brief Remove a property.
     */
    void remove(StringView name);
    
    /**
     * @brief Clear all properties.
     */
    void clear();
    
    /**
     * @brief Get all properties.
     */
    const std::unordered_map<String, StyleValue>& properties() const { return m_properties; }
    
    // Convenience setters
    Style& background(Color color);
    Style& foreground(Color color);
    Style& margin(Thickness value);
    Style& padding(Thickness value);
    Style& width(f32 value);
    Style& height(f32 value);
    Style& fontSize(f32 value);
    Style& opacity(f32 value);
    Style& visibility(Visibility value);

private:
    std::unordered_map<String, StyleValue> m_properties;
};

} // namespace gut


// --- gut/style/StyleSheet.h ---


#include <vector>

namespace gut {

// Forward declarations
class Element;

/**
 * @brief Collection of style rules that can be applied to a UI tree.
 * 
 * StyleSheet is similar to a CSS stylesheet - it contains rules that
 * match elements based on selectors and apply property values.
 */
class GUT_API StyleSheet : public Object {
    GUT_OBJECT(StyleSheet, Object)
    
public:
    StyleSheet() = default;
    ~StyleSheet() override = default;
    
    /**
     * @brief Parse a stylesheet from CSS-like syntax.
     */
    static Ref<StyleSheet> parse(StringView css);
    
    /**
     * @brief Load a stylesheet from a file.
     */
    static Ref<StyleSheet> loadFromFile(StringView path);
    
    /**
     * @brief Add a style rule.
     */
    void addRule(StyleRule rule);
    
    /**
     * @brief Remove all rules.
     */
    void clear();
    
    /**
     * @brief Get all rules.
     */
    const std::vector<StyleRule>& rules() const { return m_rules; }
    
    /**
     * @brief Get matching rules for an element, sorted by specificity.
     */
    std::vector<const StyleRule*> matchingRules(const Element& element) const;
    
    /**
     * @brief Compute the effective style for an element.
     * Merges all matching rules by specificity.
     */
    Ref<Style> computeStyle(const Element& element) const;

private:
    std::vector<StyleRule> m_rules;
};

/**
 * @brief Theme containing multiple stylesheets and named resources.
 */
class GUT_API Theme : public Object {
    GUT_OBJECT(Theme, Object)
    
public:
    Theme() = default;
    ~Theme() override = default;
    
    /**
     * @brief Add a stylesheet to the theme.
     */
    void addStyleSheet(Ref<StyleSheet> stylesheet);
    
    /**
     * @brief Get all stylesheets.
     */
    const std::vector<Ref<StyleSheet>>& styleSheets() const { return m_styleSheets; }
    
    /**
     * @brief Set a named color.
     */
    void setColor(StringView name, Color color);
    
    /**
     * @brief Get a named color.
     */
    std::optional<Color> getColor(StringView name) const;
    
    /**
     * @brief Compute effective style for an element from all stylesheets.
     */
    Ref<Style> computeStyle(const Element& element) const;
    
    /**
     * @brief Create a default light theme.
     */
    static Ref<Theme> defaultLight();
    
    /**
     * @brief Create a default dark theme.
     */
    static Ref<Theme> defaultDark();

private:
    std::vector<Ref<StyleSheet>> m_styleSheets;
    std::unordered_map<String, Color> m_colors;
};

} // namespace gut


// --- gut/resources/ResourceManager.h ---


#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>

namespace gut {

class Texture;
class Font;

/// Resource load status
enum class ResourceStatus {
    NotLoaded,
    Loading,
    Loaded,
    Failed
};

/// Base class for loadable resources
class GUT_API Resource : public Object {
    GUT_OBJECT(Resource, Object)
    
public:
    Resource();
    virtual ~Resource();
    
    /// Resource identifier/path
    const std::string& id() const { return m_id; }
    void setId(const std::string& id) { m_id = id; }
    
    /// Loading status
    ResourceStatus status() const { return m_status; }
    
    /// Error message if loading failed
    const std::string& errorMessage() const { return m_errorMessage; }
    
    /// Signals
    Signal<> loaded;
    Signal<std::string> loadFailed;
    
protected:
    std::string m_id;
    ResourceStatus m_status = ResourceStatus::NotLoaded;
    std::string m_errorMessage;
};

/// Resource loader callback
using ResourceLoadCallback = std::function<void(Ref<Resource>, bool success)>;

/// Interface for custom resource loaders
class GUT_API IResourceLoader {
public:
    virtual ~IResourceLoader() = default;
    
    /// Returns the resource types this loader handles (e.g., "texture", "font")
    virtual std::vector<std::string> supportedTypes() const = 0;
    
    /// Load resource from file path
    virtual Ref<Resource> load(const std::string& path) = 0;
    
    /// Load resource from memory
    virtual Ref<Resource> loadFromMemory(const void* data, size_t size, const std::string& hint) = 0;
};

/// Central resource manager
class GUT_API ResourceManager : public Object {
    GUT_OBJECT(ResourceManager, Object)
    
public:
    ResourceManager();
    ~ResourceManager();
    
    /// Get the global resource manager instance
    static ResourceManager* global();
    
    /// Register a custom resource loader
    void registerLoader(const std::string& type, std::unique_ptr<IResourceLoader> loader);
    
    /// Set the base path for relative resource paths
    void setBasePath(const std::string& path);
    const std::string& basePath() const { return m_basePath; }
    
    /// Load a texture by path
    Ref<Texture> loadTexture(const std::string& path);
    
    /// Load a texture from memory
    Ref<Texture> loadTextureFromMemory(const void* data, size_t size, const std::string& id = "");
    
    /// Load a font by path
    Ref<Font> loadFont(const std::string& path);
    
    /// Load a font from memory
    Ref<Font> loadFontFromMemory(const void* data, size_t size, const std::string& id = "");
    
    /// Get a cached resource by ID
    template<typename T>
    Ref<T> get(const std::string& id) const {
        auto it = m_resources.find(id);
        if (it != m_resources.end()) {
            return staticPtrCast<T>(it->second);
        }
        return nullptr;
    }
    
    /// Check if resource is loaded
    bool has(const std::string& id) const;
    
    /// Unload a specific resource
    void unload(const std::string& id);
    
    /// Unload all resources
    void unloadAll();
    
    /// Get memory usage statistics
    struct MemoryStats {
        size_t textureMemory = 0;
        size_t fontMemory = 0;
        size_t totalResources = 0;
    };
    MemoryStats memoryStats() const;
    
private:
    std::string resolvePath(const std::string& path) const;
    void cacheResource(const std::string& id, Ref<Resource> resource);
    
    std::string m_basePath;
    std::unordered_map<std::string, Ref<Resource>> m_resources;
    std::unordered_map<std::string, std::unique_ptr<IResourceLoader>> m_loaders;
};

} // namespace gut


// --- gut/resources/Font.h ---


#include <string>
#include <vector>
#include <unordered_map>

namespace gut {

/// Font weight
enum class FontWeight {
    Thin = 100,
    ExtraLight = 200,
    Light = 300,
    Normal = 400,
    Medium = 500,
    SemiBold = 600,
    Bold = 700,
    ExtraBold = 800,
    Black = 900
};

/// Font style
enum class FontStyle {
    Normal,
    Italic,
    Oblique
};

/// Glyph information
struct Glyph {
    u32 codepoint = 0;      // Unicode codepoint
    f32 advance = 0;        // Horizontal advance
    f32 bearingX = 0;       // Left side bearing
    f32 bearingY = 0;       // Top side bearing
    f32 width = 0;          // Glyph width
    f32 height = 0;         // Glyph height
    f32 u0 = 0, v0 = 0;     // Texture coordinates (top-left)
    f32 u1 = 0, v1 = 0;     // Texture coordinates (bottom-right)
};

class Texture;

/// Font face for a specific size
class GUT_API FontFace : public Object {
    GUT_OBJECT(FontFace, Object)
    
public:
    FontFace();
    ~FontFace();
    
    /// Get font size in pixels
    f32 size() const { return m_size; }
    
    /// Get line height
    f32 lineHeight() const { return m_lineHeight; }
    
    /// Get ascender (distance from baseline to top)
    f32 ascender() const { return m_ascender; }
    
    /// Get descender (distance from baseline to bottom, negative)
    f32 descender() const { return m_descender; }
    
    /// Get glyph for codepoint
    const Glyph* glyph(u32 codepoint) const;
    
    /// Get kerning between two glyphs
    f32 kerning(u32 left, u32 right) const;
    
    /// Get the font atlas texture
    Texture* atlasTexture() const { return m_atlas.get(); }
    
    /// Get raw atlas bitmap (single-channel, for backend upload)
    const u8* atlasBitmap() const { return m_atlasBitmap.data(); }
    
    /// Get atlas dimensions
    i32 atlasWidth() const { return m_atlasWidth; }
    i32 atlasHeight() const { return m_atlasHeight; }
    
    /// Check if atlas needs GPU upload
    bool needsUpload() const { return m_needsUpload; }
    
    /// Mark atlas as uploaded (call after backend creates texture)
    void markUploaded() { m_needsUpload = false; }
    
    /// Measure text width
    f32 measureWidth(const std::string& text) const;
    
    /// Measure text dimensions
    Size2f measureText(const std::string& text, f32 maxWidth = 0) const;
    
private:
    friend class Font;
    friend class RenderContext;
    
    f32 m_size = 0;
    f32 m_lineHeight = 0;
    f32 m_ascender = 0;
    f32 m_descender = 0;
    
    // Atlas texture (GPU-side, created by backend)
    Ref<Texture> m_atlas;
    
    // Atlas bitmap (CPU-side, for backend upload)
    std::vector<u8> m_atlasBitmap;
    i32 m_atlasWidth = 0;
    i32 m_atlasHeight = 0;
    bool m_needsUpload = true;
    
    std::unordered_map<u32, Glyph> m_glyphs;
    std::unordered_map<u64, f32> m_kerning; // (left << 32 | right) -> kerning
};

/// Font resource
class GUT_API Font : public Resource {
    GUT_OBJECT(Font, Resource)
    
public:
    Font();
    ~Font();
    
    /// Load font from memory (raw .ttf/.otf data)
    bool loadFromMemory(const u8* data, size_t size);
    
    /// Check if font is loaded
    bool isLoaded() const { return !m_fontData.empty(); }
    
    /// Get font family name
    const std::string& family() const { return m_family; }
    
    /// Get font weight
    FontWeight weight() const { return m_weight; }
    
    /// Get font style
    FontStyle style() const { return m_style; }
    
    /// Set font family name (overrides auto-detected value)
    void setFamily(const std::string& family) { m_family = family; }
    
    /// Set font weight (overrides auto-detected value)
    void setWeight(FontWeight weight) { m_weight = weight; }
    
    /// Set font style (overrides auto-detected value)
    void setStyle(FontStyle style) { m_style = style; }
    
    /// Check if font has a specific glyph
    bool hasGlyph(u32 codepoint) const;
    
    /// Get or create a font face at the specified size
    Ref<FontFace> getFace(f32 size, RenderBackend* backend = nullptr);
    
    /// Preload font at specific sizes
    void preload(const std::vector<f32>& sizes, RenderBackend* backend = nullptr);
    
    /// Clear cached font faces to free memory
    void clearCache();
    
private:
    friend class ResourceManager;
    friend class Context;
    
    bool initStbFont();
    
    std::string m_family;
    FontWeight m_weight = FontWeight::Normal;
    FontStyle m_style = FontStyle::Normal;
    
    // Raw font data for rasterization
    std::vector<u8> m_fontData;
    bool m_stbInitialized = false;
    
    // Cached font faces by size
    std::unordered_map<i32, Ref<FontFace>> m_faces; // keyed by size*10 for precision
};

/// Text shaping result
struct ShapedGlyph {
    const Glyph* glyph = nullptr;
    f32 x = 0;
    f32 y = 0;
};

/// Text shaping for complex text layout
class GUT_API TextShaper {
public:
    TextShaper();
    ~TextShaper();
    
    /// Shape text using the given font face
    std::vector<ShapedGlyph> shape(FontFace* face, const std::string& text) const;
    
    /// Shape text with line wrapping
    struct ShapedLine {
        std::vector<ShapedGlyph> glyphs;
        f32 width = 0;
    };
    
    std::vector<ShapedLine> shapeMultiline(FontFace* face, const std::string& text, 
                                            f32 maxWidth, TextAlignment alignment = TextAlignment::Left) const;
    
private:
    // Break text into words for wrapping
    std::vector<std::string> breakWords(const std::string& text) const;
};

} // namespace gut


// --- gut/elements/Element.h ---


#include <vector>

namespace gut {

// Forward declarations
class Panel;
class Style;
class RenderContext;
struct MouseEvent;
struct KeyEvent;

/**
 * @brief Base class for all UI elements.
 * 
 * Element provides:
 * - Position and size
 * - Parent-child relationships
 * - Layout participation (measure/arrange)
 * - Input hit testing
 * - Style support
 * - Visibility and enabled state
 */
class GUT_API Element : public Object {
    GUT_OBJECT(Element, Object)
    
public:
    Element();
    ~Element() override;
    
    // -------------------------------------------------------------------------
    // Identification
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(String, id, "")
    GUT_PROPERTY(String, className, "")
    
    // -------------------------------------------------------------------------
    // Layout properties
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(f32, width, NAN)         // Explicit width (NAN = auto)
    GUT_PROPERTY(f32, height, NAN)        // Explicit height (NAN = auto)
    GUT_PROPERTY(f32, minWidth, 0.0f)
    GUT_PROPERTY(f32, minHeight, 0.0f)
    GUT_PROPERTY(f32, maxWidth, INFINITY)
    GUT_PROPERTY(f32, maxHeight, INFINITY)
    
    GUT_PROPERTY(Thickness, margin, Thickness{})
    GUT_PROPERTY(Thickness, padding, Thickness{})
    
    GUT_PROPERTY(HorizontalAlignment, horizontalAlignment, HorizontalAlignment::Stretch)
    GUT_PROPERTY(VerticalAlignment, verticalAlignment, VerticalAlignment::Stretch)
    
    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(Visibility, visibility, Visibility::Visible)
    GUT_PROPERTY(f32, opacity, 1.0f)
    GUT_PROPERTY(f32, scaleX, 1.0f)
    GUT_PROPERTY(f32, scaleY, 1.0f)
    GUT_PROPERTY(f32, rotation, 0.0f)
    GUT_PROPERTY(f32, skewX, 0.0f)
    GUT_PROPERTY(f32, skewY, 0.0f)
    GUT_PROPERTY(bool, isEnabled, true)
    GUT_PROPERTY(i32, zIndex, 0)
    GUT_PROPERTY(bool, clipToBounds, false)
    GUT_PROPERTY(CursorType, cursor, CursorType::Arrow)
    
    // -------------------------------------------------------------------------
    // Layout results (read-only after layout)
    // -------------------------------------------------------------------------
    
    /**
     * @brief Get the computed bounds (position + size) in parent coordinates.
     */
    const Rectf& bounds() const { return m_bounds; }
    
    /**
     * @brief Get the desired size computed during measure pass.
     */
    const Size2f& desiredSize() const { return m_desiredSize; }
    
    /**
     * @brief Get bounds in screen/root coordinates.
     */
    Rectf screenBounds() const;
    
    // -------------------------------------------------------------------------
    // Tree structure
    // -------------------------------------------------------------------------
    
    /**
     * @brief Get the parent element (nullptr if root).
     */
    Element* parent() const { return m_parent; }
    
    /**
     * @brief Check if this element has any children.
     */
    virtual bool hasChildren() const { return false; }
    
    /**
     * @brief Get the number of children.
     */
    virtual usize childCount() const { return 0; }
    
    /**
     * @brief Get a child by index.
     */
    virtual Element* childAt(usize index) const { (void)index; return nullptr; }
    
    /**
     * @brief Find an element by ID in this subtree.
     */
    Element* findById(StringView id);
    
    /**
     * @brief Find all elements matching a class name.
     */
    std::vector<Element*> findByClass(StringView className);
    
    // -------------------------------------------------------------------------
    // Layout
    // -------------------------------------------------------------------------
    
    /**
     * @brief Measure pass: compute desired size given available space.
     * @param availableSize The available space from the parent.
     * @return The desired size of this element.
     */
    Size2f measure(Size2f availableSize);
    
    /**
     * @brief Arrange pass: position and size the element.
     * @param finalRect The final bounds assigned by the parent.
     */
    void arrange(Rectf finalRect);
    
    /**
     * @brief Mark layout as dirty (needs re-measure/arrange).
     */
    void invalidateLayout();
    
    /**
     * @brief Mark rendering as dirty (needs repaint).
     */
    void invalidateRender();
    
    // -------------------------------------------------------------------------
    // Attached Properties (generic system for Grid, Canvas, etc.)
    // -------------------------------------------------------------------------
    
    /// Set an attached property (type-safe via std::any)
    template<typename T>
    void setAttachedProperty(StringView key, T value) {
        m_attachedProperties[std::string(key)] = std::any(std::move(value));
    }
    
    /// Get an attached property, returns defaultValue if not set
    template<typename T>
    T getAttachedProperty(StringView key, T defaultValue = T{}) const {
        auto it = m_attachedProperties.find(std::string(key));
        if (it == m_attachedProperties.end()) return defaultValue;
        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast&) {
            return defaultValue;
        }
    }
    
    /// Check if an attached property exists
    bool hasAttachedProperty(StringView key) const {
        return m_attachedProperties.find(std::string(key)) != m_attachedProperties.end();
    }
    
    // -------------------------------------------------------------------------
    // Rendering
    // -------------------------------------------------------------------------
    
    /**
     * @brief Render this element and its children.
     * @param ctx The render context.
     */
    virtual void render(RenderContext& ctx);
    
    // -------------------------------------------------------------------------
    // Input
    // -------------------------------------------------------------------------
    
    /**
     * @brief Hit test: find the element at the given point.
     * @param point Point in this element's coordinate space.
     * @return The element at the point, or nullptr.
     */
    virtual Element* hitTest(Point2f point);
    
    /**
     * @brief Check if a point is within this element's bounds.
     */
    bool containsPoint(Point2f point) const;
    
    /**
     * @brief Handle a mouse event.
     * @return true if the event was handled.
     */
    virtual bool onMouseEvent(const MouseEvent& event);
    
    /**
     * @brief Handle a keyboard event.
     * @return true if the event was handled.
     */
    virtual bool onKeyEvent(const KeyEvent& event);
    
    // -------------------------------------------------------------------------
    // Focus
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(bool, focusable, false)
    GUT_PROPERTY(i32, tabIndex, 0)
    GUT_PROPERTY(bool, isHitTestVisible, true)
    GUT_PROPERTY_READONLY(bool, isFocused, false)
    GUT_PROPERTY_READONLY(bool, isHovered, false)
    GUT_PROPERTY_READONLY(bool, isPressed, false)
    
    /**
     * @brief Request focus for this element.
     */
    void focus();
    
    /**
     * @brief Release focus from this element.
     */
    void unfocus();
    
    // -------------------------------------------------------------------------
    // Signals
    // -------------------------------------------------------------------------
    
    Signal<>& clicked() { return m_clicked; }
    Signal<>& doubleClicked() { return m_doubleClicked; }
    Signal<>& focusGained() { return m_focusGained; }
    Signal<>& focusLost() { return m_focusLost; }
    Signal<>& mouseEntered() { return m_mouseEntered; }
    Signal<>& mouseLeft() { return m_mouseLeft; }

protected:
    // -------------------------------------------------------------------------
    // Virtual methods for subclasses
    // -------------------------------------------------------------------------
    
    /**
     * @brief Override to compute the desired size of this element.
     * @param availableSize Available space from parent.
     * @return The desired size.
     */
    virtual Size2f measureOverride(Size2f availableSize);
    
    /**
     * @brief Override to position children within the final bounds.
     * @param finalSize The final size of this element.
     * @return The actual size used.
     */
    virtual Size2f arrangeOverride(Size2f finalSize);
    
    /**
     * @brief Override to render this element.
     * @param ctx The render context.
     */
    virtual void onRender(RenderContext& ctx);
    
    /**
     * @brief Get corner radius for clipping. Override in Panel.
     */
    virtual f32 getClipCornerRadius() const { return 0.0f; }
    
    /**
     * @brief Called when focus is gained.
     */
    virtual void onFocusGained();
    
    /**
     * @brief Called when focus is lost.
     */
    virtual void onFocusLost();
    
    /**
     * @brief Called when mouse enters this element.
     */
    virtual void onMouseEnter();
    
    /**
     * @brief Called when mouse leaves this element.
     */
    virtual void onMouseLeave();

    /**
     * @brief Set the parent element (called by Panel).
     */
    void setParent(Element* parent);
    
    friend class Panel;
    friend class Context;
    friend class InputManager;
    friend class FocusManager;
    friend class Tooltip;
    friend class ContextMenu;
    friend class ListView;

    /// Get the owning context (set when element is added to a rooted tree)
    Context* context() const { return m_context; }
    void setContext(Context* ctx);

private:
    Element* m_parent{nullptr};
    Context* m_context{nullptr};
    Rectf m_bounds{};
    std::unordered_map<std::string, std::any> m_attachedProperties;
    Size2f m_desiredSize{};
    bool m_layoutDirty{true};
    bool m_renderDirty{true};
    
    Signal<> m_clicked;
    Signal<> m_doubleClicked;
    Signal<> m_focusGained;
    Signal<> m_focusLost;
    Signal<> m_mouseEntered;
    Signal<> m_mouseLeft;
};

} // namespace gut


// --- gut/elements/Panel.h ---


#include <vector>
#include <initializer_list>

namespace gut {

/**
 * @brief Base class for elements that contain children.
 * 
 * Panel provides:
 * - Child management (add, remove, clear)
 * - Child hit testing
 * - Child rendering
 * 
 * Subclasses implement layout strategies (StackPanel, Grid, Canvas, etc.)
 */
class GUT_API Panel : public Element {
    GUT_OBJECT(Panel, Element)
    
public:
    Panel() = default;
    explicit Panel(std::initializer_list<Ref<Element>> children);
    ~Panel() override;
    
    // -------------------------------------------------------------------------
    // Background
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(Color, background, Color::transparent())
    GUT_PROPERTY(Color, backgroundGradientTop, Color::transparent())
    GUT_PROPERTY(Color, backgroundGradientBottom, Color::transparent())
    GUT_PROPERTY(Color, hoverBackground, Color::transparent())
    GUT_PROPERTY(Color, pressedBackground, Color::transparent())

    /// Optional Brush for the background (takes precedence over solid/gradient colors).
    Ref<Brush> backgroundBrush() const { return m_backgroundBrush; }
    void setbackgroundBrush(Ref<Brush> b) { m_backgroundBrush = std::move(b); }
    GUT_PROPERTY(f32, cornerRadius, 0.0f)
    GUT_PROPERTY(Color, borderColor, Color::transparent())
    GUT_PROPERTY(f32, borderWidth, 0.0f)
    
    // -------------------------------------------------------------------------
    // Drop Shadow
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(Color, shadowColor, Color::transparent())
    GUT_PROPERTY(f32, shadowOffsetX, 0.0f)
    GUT_PROPERTY(f32, shadowOffsetY, 4.0f)
    GUT_PROPERTY(f32, shadowBlurRadius, 8.0f)
    
    // -------------------------------------------------------------------------
    // Inset Shadow (inner shadow)
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(Color, insetShadowColor, Color::transparent())
    GUT_PROPERTY(f32, insetShadowOffsetX, 0.0f)
    GUT_PROPERTY(f32, insetShadowOffsetY, 0.0f)
    GUT_PROPERTY(f32, insetShadowBlurRadius, 8.0f)
    
    // -------------------------------------------------------------------------
    // Backdrop Blur (frosted glass)
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(f32, backdropBlur, 0.0f)
    GUT_PROPERTY(Color, backdropTint, Color::transparent())
    
    void setOnClick(std::function<void()> callback) { m_onClick = std::move(callback); }
    
    // -------------------------------------------------------------------------
    // Child management
    // -------------------------------------------------------------------------
    
    /**
     * @brief Add a child element.
     */
    void addChild(Ref<Element> child);
    
    /**
     * @brief Insert a child at a specific index.
     */
    void insertChild(usize index, Ref<Element> child);
    
    /**
     * @brief Remove a child element.
     * @return true if the child was found and removed.
     */
    bool removeChild(Element* child);
    
    /**
     * @brief Remove a child by index.
     */
    void removeChildAt(usize index);
    
    /**
     * @brief Remove all children.
     */
    void clearChildren();
    
    /**
     * @brief Get all children.
     */
    const std::vector<Ref<Element>>& children() const { return m_children; }
    
    // -------------------------------------------------------------------------
    // Element overrides
    // -------------------------------------------------------------------------
    
    bool hasChildren() const override { return !m_children.empty(); }
    usize childCount() const override { return m_children.size(); }
    Element* childAt(usize index) const override;
    
    Element* hitTest(Point2f point) override;

protected:
    Size2f measureOverride(Size2f availableSize) override;
    Size2f arrangeOverride(Size2f finalSize) override;
    void onRender(RenderContext& ctx) override;
    f32 getClipCornerRadius() const override { return cornerRadius(); }
    void onMouseEnter() override;
    void onMouseLeave() override;
    bool onMouseEvent(const MouseEvent& event) override;
    
    /**
     * @brief Render children (called by onRender after background).
     */
    virtual void renderChildren(RenderContext& ctx);
    
    std::vector<Ref<Element>> m_children;
    Ref<Brush> m_backgroundBrush;
    std::function<void()> m_onClick;
};

} // namespace gut


// --- gut/elements/StackPanel.h ---



namespace gut {

/**
 * @brief Panel that arranges children in a single line (horizontal or vertical).
 */
class GUT_API StackPanel : public Panel {
    GUT_OBJECT(StackPanel, Panel)
    
public:
    StackPanel() = default;
    explicit StackPanel(Orientation orientation);
    StackPanel(Orientation orientation, std::initializer_list<Ref<Element>> children);
    ~StackPanel() override = default;
    
    GUT_PROPERTY(Orientation, orientation, Orientation::Vertical)
    GUT_PROPERTY(f32, spacing, 0.0f)

protected:
    Size2f measureOverride(Size2f availableSize) override;
    Size2f arrangeOverride(Size2f finalSize) override;
};

} // namespace gut


// --- gut/elements/Grid.h ---


#include <vector>

namespace gut {

/**
 * @brief Defines a row or column in a Grid.
 */
struct GUT_API GridLength {
    enum class Type {
        Auto,       // Size to content
        Pixel,      // Fixed pixel size
        Star        // Proportional (weighted)
    };
    
    Type type{Type::Star};
    f32 value{1.0f};
    
    constexpr GridLength() = default;
    constexpr GridLength(Type t, f32 v = 1.0f) : type(t), value(v) {}
    
    static constexpr GridLength Auto() { return {Type::Auto}; }
    static constexpr GridLength Pixel(f32 pixels) { return {Type::Pixel, pixels}; }
    static constexpr GridLength Star(f32 weight = 1.0f) { return {Type::Star, weight}; }
};

/**
 * @brief Row definition for Grid.
 */
struct GUT_API RowDefinition {
    GridLength height{GridLength::Star()};
    f32 minHeight{0.0f};
    f32 maxHeight{INFINITY};
    
    // Computed during layout
    f32 actualHeight{0.0f};
};

/**
 * @brief Column definition for Grid.
 */
struct GUT_API ColumnDefinition {
    GridLength width{GridLength::Star()};
    f32 minWidth{0.0f};
    f32 maxWidth{INFINITY};
    
    // Computed during layout
    f32 actualWidth{0.0f};
};

/**
 * @brief Panel that arranges children in rows and columns.
 */
class GUT_API Grid : public Panel {
    GUT_OBJECT(Grid, Panel)
    
public:
    Grid() = default;
    Grid(usize rows, usize columns);
    ~Grid() override = default;
    
    // -------------------------------------------------------------------------
    // Row/Column definitions
    // -------------------------------------------------------------------------
    
    void setRowDefinitions(std::vector<RowDefinition> rows);
    void setColumnDefinitions(std::vector<ColumnDefinition> columns);
    
    const std::vector<RowDefinition>& rowDefinitions() const { return m_rows; }
    const std::vector<ColumnDefinition>& columnDefinitions() const { return m_columns; }
    
    usize rowCount() const { return m_rows.empty() ? 1 : m_rows.size(); }
    usize columnCount() const { return m_columns.empty() ? 1 : m_columns.size(); }
    
    // -------------------------------------------------------------------------
    // Attached properties for children
    // -------------------------------------------------------------------------
    
    /**
     * @brief Set the row index for a child element.
     */
    static void setRow(Element& element, usize row);
    static usize getRow(const Element& element);
    
    /**
     * @brief Set the column index for a child element.
     */
    static void setColumn(Element& element, usize column);
    static usize getColumn(const Element& element);
    
    /**
     * @brief Set row span for a child element.
     */
    static void setRowSpan(Element& element, usize span);
    static usize getRowSpan(const Element& element);
    
    /**
     * @brief Set column span for a child element.
     */
    static void setColumnSpan(Element& element, usize span);
    static usize getColumnSpan(const Element& element);

protected:
    Size2f measureOverride(Size2f availableSize) override;
    Size2f arrangeOverride(Size2f finalSize) override;

private:
    void computeRowHeights(f32 availableHeight);
    void computeColumnWidths(f32 availableWidth);
    Rectf getCellBounds(usize row, usize column, usize rowSpan, usize colSpan) const;
    
    std::vector<RowDefinition> m_rows;
    std::vector<ColumnDefinition> m_columns;
};

} // namespace gut


// --- gut/elements/Canvas.h ---



namespace gut {

/**
 * @brief Panel that positions children at absolute coordinates.
 * 
 * Children are positioned using attached Left, Top, Right, Bottom properties.
 */
class GUT_API Canvas : public Panel {
    GUT_OBJECT(Canvas, Panel)
    
public:
    Canvas() = default;
    ~Canvas() override = default;
    
    // -------------------------------------------------------------------------
    // Attached properties for children
    // -------------------------------------------------------------------------
    
    static void setLeft(Element& element, f32 value);
    static f32 getLeft(const Element& element);
    
    static void setTop(Element& element, f32 value);
    static f32 getTop(const Element& element);
    
    static void setRight(Element& element, f32 value);
    static f32 getRight(const Element& element);
    
    static void setBottom(Element& element, f32 value);
    static f32 getBottom(const Element& element);

protected:
    Size2f measureOverride(Size2f availableSize) override;
    Size2f arrangeOverride(Size2f finalSize) override;
};

} // namespace gut


// --- gut/elements/WrapPanel.h ---



namespace gut {

/**
 * @brief Panel that arranges children sequentially, wrapping to new lines when
 *        the available space is exhausted along the main axis.
 *
 * Similar to CSS `flex-wrap: wrap`. Children flow left-to-right (Horizontal)
 * or top-to-bottom (Vertical), and a new row/column is started when the next
 * child would exceed the panel width/height.
 */
class GUT_API WrapPanel : public Panel {
    GUT_OBJECT(WrapPanel, Panel)

public:
    WrapPanel() = default;
    explicit WrapPanel(Orientation orientation);
    ~WrapPanel() override = default;

    GUT_PROPERTY(Orientation, orientation, Orientation::Horizontal)
    GUT_PROPERTY(f32, itemSpacing, 0.0f)   ///< Spacing between items in a line
    GUT_PROPERTY(f32, lineSpacing, 0.0f)   ///< Spacing between lines

protected:
    Size2f measureOverride(Size2f availableSize) override;
    Size2f arrangeOverride(Size2f finalSize) override;
};

} // namespace gut


// --- gut/elements/DockPanel.h ---



namespace gut {

/**
 * @brief Dock direction for DockPanel children.
 */
enum class Dock : u8 {
    Left,
    Top,
    Right,
    Bottom
};

/**
 * @brief Panel that docks children to the edges of the available space.
 *
 * Each child is docked to a side (Left, Top, Right, Bottom) and the remaining
 * space shrinks accordingly. By default, the last child fills the remaining
 * space. Set `lastChildFill(false)` to dock it like the others.
 */
class GUT_API DockPanel : public Panel {
    GUT_OBJECT(DockPanel, Panel)

public:
    DockPanel() = default;
    ~DockPanel() override = default;

    GUT_PROPERTY(bool, lastChildFill, true)

    // Attached property — set Dock on a child element
    static void setDock(Element& element, Dock value);
    static Dock getDock(const Element& element);

protected:
    Size2f measureOverride(Size2f availableSize) override;
    Size2f arrangeOverride(Size2f finalSize) override;
};

} // namespace gut


// --- gut/elements/ViewBox.h ---



namespace gut {

/**
 * @brief Stretch mode for ViewBox.
 */
enum class ViewBoxStretch : u8 {
    None,           ///< Child at natural size, no scaling
    Uniform,        ///< Scale to fit, preserving aspect ratio
    UniformToFill,  ///< Scale to fill, preserving aspect ratio (may clip)
    Fill            ///< Stretch to fill exactly (may distort)
};

/**
 * @brief Container that scales its single child to fit the available space.
 *
 * Measures the child at infinite size, then applies a scale transform so that
 * the child fits within the ViewBox bounds according to the stretch mode.
 */
class GUT_API ViewBox : public Panel {
    GUT_OBJECT(ViewBox, Panel)

public:
    ViewBox() = default;
    ~ViewBox() override = default;

    GUT_PROPERTY(ViewBoxStretch, stretch, ViewBoxStretch::Uniform)

protected:
    Size2f measureOverride(Size2f availableSize) override;
    Size2f arrangeOverride(Size2f finalSize) override;
    void renderChildren(RenderContext& ctx) override;

private:
    f32 m_scaleX = 1.0f;
    f32 m_scaleY = 1.0f;
    f32 m_offsetX = 0.0f;
    f32 m_offsetY = 0.0f;
};

} // namespace gut


// --- gut/elements/AnchorPanel.h ---



namespace gut {

/**
 * @brief Panel that positions children using anchor constraints relative to
 *        the panel edges (or a fraction of the panel size).
 *
 * Extends Canvas-style absolute positioning with proportional anchors.
 * Each child can have AnchorLeft/AnchorTop/AnchorRight/AnchorBottom (0..1)
 * specifying which fraction of the panel edge to anchor to, plus Offset
 * values (Left/Top/Right/Bottom like Canvas).
 *
 * If both left and right offsets are set, the child is stretched horizontally.
 * If both top and bottom offsets are set, the child is stretched vertically.
 */
class GUT_API AnchorPanel : public Panel {
    GUT_OBJECT(AnchorPanel, Panel)

public:
    AnchorPanel() = default;
    ~AnchorPanel() override = default;

    // -------------------------------------------------------------------------
    // Attached properties — anchor ratios (0..1, fraction of panel size)
    // -------------------------------------------------------------------------
    static void setAnchorLeft(Element& element, f32 value);
    static f32  getAnchorLeft(const Element& element);
    static void setAnchorTop(Element& element, f32 value);
    static f32  getAnchorTop(const Element& element);
    static void setAnchorRight(Element& element, f32 value);
    static f32  getAnchorRight(const Element& element);
    static void setAnchorBottom(Element& element, f32 value);
    static f32  getAnchorBottom(const Element& element);

    // -------------------------------------------------------------------------
    // Attached properties — offsets from the anchor point (pixels)
    // -------------------------------------------------------------------------
    static void setOffsetLeft(Element& element, f32 value);
    static f32  getOffsetLeft(const Element& element);
    static void setOffsetTop(Element& element, f32 value);
    static f32  getOffsetTop(const Element& element);
    static void setOffsetRight(Element& element, f32 value);
    static f32  getOffsetRight(const Element& element);
    static void setOffsetBottom(Element& element, f32 value);
    static f32  getOffsetBottom(const Element& element);

protected:
    Size2f measureOverride(Size2f availableSize) override;
    Size2f arrangeOverride(Size2f finalSize) override;
};

} // namespace gut


// --- gut/elements/UniformGrid.h ---



namespace gut {

/**
 * @brief Grid where every cell has the same size.
 *
 * Set `columns` (or `rows`) and the panel computes the other dimension
 * automatically from the child count.  All cells are sized equally.
 */
class GUT_API UniformGrid : public Panel {
    GUT_OBJECT(UniformGrid, Panel)

public:
    UniformGrid() = default;
    ~UniformGrid() override = default;

    /// Number of columns (0 = auto-compute from rows + child count).
    GUT_PROPERTY(i32, columns, 0)
    /// Number of rows (0 = auto-compute from columns + child count).
    GUT_PROPERTY(i32, rows, 0)

protected:
    Size2f measureOverride(Size2f availableSize) override;
    Size2f arrangeOverride(Size2f finalSize) override;

private:
    void computeGrid(i32& outCols, i32& outRows) const;
};

} // namespace gut


// --- gut/elements/FlexPanel.h ---



namespace gut {

/**
 * @brief Justify-content modes for FlexPanel.
 */
enum class FlexJustify : u8 {
    Start,          ///< Pack children to the start
    End,            ///< Pack children to the end
    Center,         ///< Center children
    SpaceBetween,   ///< Equal space between children
    SpaceAround,    ///< Equal space around children
    SpaceEvenly     ///< Equal space between and at edges
};

/**
 * @brief Cross-axis alignment for FlexPanel.
 */
enum class FlexAlign : u8 {
    Start,          ///< Align to cross-axis start
    End,            ///< Align to cross-axis end
    Center,         ///< Center on cross-axis
    Stretch         ///< Stretch to fill cross-axis
};

/**
 * @brief CSS Flexbox-style layout panel.
 *
 * Children flow along the main axis (horizontal or vertical).
 * Each child can have attached flex-grow / flex-shrink / flex-basis
 * to control how remaining space is distributed or overflows handled.
 */
class GUT_API FlexPanel : public Panel {
    GUT_OBJECT(FlexPanel, Panel)

public:
    FlexPanel() = default;
    explicit FlexPanel(Orientation orientation);
    ~FlexPanel() override = default;

    GUT_PROPERTY(Orientation, orientation, Orientation::Horizontal)
    GUT_PROPERTY(f32, spacing, 0.0f)
    GUT_PROPERTY(FlexJustify, justifyContent, FlexJustify::Start)
    GUT_PROPERTY(FlexAlign, alignItems, FlexAlign::Stretch)

    // Attached properties per child
    static void setFlexGrow(Element& element, f32 value);
    static f32  getFlexGrow(const Element& element);
    static void setFlexShrink(Element& element, f32 value);
    static f32  getFlexShrink(const Element& element);
    static void setFlexBasis(Element& element, f32 value);  ///< NAN = use natural size
    static f32  getFlexBasis(const Element& element);
    static void setAlignSelf(Element& element, FlexAlign value);
    static FlexAlign getAlignSelf(const Element& element); ///< returns FlexAlign::Stretch for "inherit"

protected:
    Size2f measureOverride(Size2f availableSize) override;
    Size2f arrangeOverride(Size2f finalSize) override;
};

} // namespace gut


// --- gut/elements/RelativePanel.h ---



namespace gut {

/**
 * @brief UWP-style RelativePanel that positions children relative to each
 *        other or to the panel edges.
 *
 * Children reference siblings by id (Element::id()).  Attached properties
 * such as `RightOf`, `Below`, `AlignTopWith`, `AlignHorizontalCenterWith`, etc.
 * define spatial relationships.
 */
class GUT_API RelativePanel : public Panel {
    GUT_OBJECT(RelativePanel, Panel)

public:
    RelativePanel() = default;
    ~RelativePanel() override = default;

    // --- Positional: place element adjacent to a named sibling ---
    static void setLeftOf(Element& el, const String& siblingName);
    static String getLeftOf(const Element& el);

    static void setRightOf(Element& el, const String& siblingName);
    static String getRightOf(const Element& el);

    static void setAbove(Element& el, const String& siblingName);
    static String getAbove(const Element& el);

    static void setBelow(Element& el, const String& siblingName);
    static String getBelow(const Element& el);

    // --- Alignment: align edge with a sibling's edge ---
    static void setAlignLeftWith(Element& el, const String& siblingName);
    static String getAlignLeftWith(const Element& el);

    static void setAlignTopWith(Element& el, const String& siblingName);
    static String getAlignTopWith(const Element& el);

    static void setAlignRightWith(Element& el, const String& siblingName);
    static String getAlignRightWith(const Element& el);

    static void setAlignBottomWith(Element& el, const String& siblingName);
    static String getAlignBottomWith(const Element& el);

    // --- Panel-edge alignment ---
    static void setAlignLeftWithPanel(Element& el, bool value);
    static bool getAlignLeftWithPanel(const Element& el);

    static void setAlignTopWithPanel(Element& el, bool value);
    static bool getAlignTopWithPanel(const Element& el);

    static void setAlignRightWithPanel(Element& el, bool value);
    static bool getAlignRightWithPanel(const Element& el);

    static void setAlignBottomWithPanel(Element& el, bool value);
    static bool getAlignBottomWithPanel(const Element& el);

    static void setAlignHorizontalCenterWithPanel(Element& el, bool value);
    static bool getAlignHorizontalCenterWithPanel(const Element& el);

    static void setAlignVerticalCenterWithPanel(Element& el, bool value);
    static bool getAlignVerticalCenterWithPanel(const Element& el);

protected:
    Size2f measureOverride(Size2f availableSize) override;
    Size2f arrangeOverride(Size2f finalSize) override;

private:
    Element* findSibling(const String& name) const;
};

} // namespace gut


// --- gut/elements/RadialPanel.h ---



namespace gut {

/**
 * @brief Arranges children in a circle or arc.
 *
 * Children are evenly spaced around a circle centred in the panel.
 * `startAngle` and `endAngle` (degrees) control the arc range.
 * `radius` overrides auto-radius from panel size.
 */
class GUT_API RadialPanel : public Panel {
    GUT_OBJECT(RadialPanel, Panel)

public:
    RadialPanel() = default;
    ~RadialPanel() override = default;

    GUT_PROPERTY(f32, startAngle, 0.0f)      ///< degrees, 0 = top (12 o'clock)
    GUT_PROPERTY(f32, endAngle, 360.0f)      ///< degrees, 360 = full circle
    GUT_PROPERTY(f32, radius, 0.0f)          ///< 0 = auto (half of min dimension)
    GUT_PROPERTY(bool, rotateItems, false)   ///< rotate children to face outward

protected:
    Size2f measureOverride(Size2f availableSize) override;
    Size2f arrangeOverride(Size2f finalSize) override;
};

} // namespace gut


// --- gut/elements/Text.h ---



namespace gut {

// Forward declaration
class Font;

/**
 * @brief Element that displays text.
 */
class GUT_API Text : public Element {
    GUT_OBJECT(Text, Element)
    
public:
    Text() = default;
    explicit Text(String text);
    Text(String text, f32 fontSize);
    ~Text() override = default;
    
    // -------------------------------------------------------------------------
    // Content
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(String, text, "")
    
    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(Color, foreground, Color::black())
    GUT_PROPERTY(String, fontFamily, "sans-serif")
    GUT_PROPERTY(f32, fontSize, 14.0f)
    GUT_PROPERTY(FontWeight, fontWeight, FontWeight::Normal)
    GUT_PROPERTY(FontStyle, fontStyle, FontStyle::Normal)
    GUT_PROPERTY(bool, bold, false)
    GUT_PROPERTY(bool, italic, false)
    GUT_PROPERTY(bool, underline, false)
    GUT_PROPERTY(bool, strikethrough, false)
    
    // -------------------------------------------------------------------------
    // Text Effects — Shadow / Outline (Stroke) / Glow
    // -------------------------------------------------------------------------
    
    /// Drop shadow: colour, offset, blur radius
    GUT_PROPERTY(Color, textShadowColor, Color::transparent())
    GUT_PROPERTY(f32, textShadowOffsetX, 2.0f)
    GUT_PROPERTY(f32, textShadowOffsetY, 2.0f)
    GUT_PROPERTY(f32, textShadowBlurRadius, 0.0f)
    
    /// Outline / stroke around each glyph
    GUT_PROPERTY(Color, textStrokeColor, Color::transparent())
    GUT_PROPERTY(f32, textStrokeWidth, 1.0f)
    
    /// Outer glow (similar to Photoshop outer glow)
    GUT_PROPERTY(Color, textGlowColor, Color::transparent())
    GUT_PROPERTY(f32, textGlowRadius, 4.0f)
    
    /// Resolve the effective font weight (fontWeight property, or Bold if bold()==true).
    FontWeight effectiveFontWeight() const {
        if (bold() && fontWeight() == FontWeight::Normal)
            return FontWeight::Bold;
        return fontWeight();
    }
    
    /// Resolve the effective font style (fontStyle property, or Italic if italic()==true).
    FontStyle effectiveFontStyle() const {
        if (italic() && fontStyle() == FontStyle::Normal)
            return FontStyle::Italic;
        return fontStyle();
    }
    
    // -------------------------------------------------------------------------
    // Layout
    // -------------------------------------------------------------------------
    
    enum class TextWrapping {
        NoWrap,
        Wrap,
        WrapWholeWords
    };
    
    enum class TextTrimming {
        None,
        CharacterEllipsis,
        WordEllipsis
    };
    
    enum class TextAlignment {
        Left,
        Center,
        Right,
        Justify
    };
    
    GUT_PROPERTY(TextWrapping, textWrapping, TextWrapping::NoWrap)
    GUT_PROPERTY(TextTrimming, textTrimming, TextTrimming::None)
    GUT_PROPERTY(TextAlignment, textAlignment, TextAlignment::Left)
    GUT_PROPERTY(f32, lineHeight, 1.2f)  // Multiplier of font size

protected:
    Size2f measureOverride(Size2f availableSize) override;
    void onRender(RenderContext& ctx) override;
};

} // namespace gut


// --- gut/elements/Button.h ---


#include <functional>

namespace gut {

/**
 * @brief Clickable button element.
 */
class GUT_API Button : public Element {
    GUT_OBJECT(Button, Element)
    
public:
    Button() = default;
    explicit Button(String label);
    Button(String label, std::function<void()> onClick);
    ~Button() override = default;
    
    // -------------------------------------------------------------------------
    // Content
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(String, label, "")
    
    /**
     * @brief Set custom content (overrides label).
     */
    void setContent(Ref<Element> content);
    Element* content() const { return m_content.get(); }
    
    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(Color, background, Color::fromHex(0xE0E0E0))
    GUT_PROPERTY(Color, foreground, Color::black())
    GUT_PROPERTY(Color, hoverBackground, Color::fromHex(0xD0D0D0))
    GUT_PROPERTY(Color, pressedBackground, Color::fromHex(0xC0C0C0))
    GUT_PROPERTY(Color, disabledBackground, Color::fromHex(0xF0F0F0))
    GUT_PROPERTY(Color, borderColor, Color::fromHex(0xA0A0A0))
    GUT_PROPERTY(Color, focusBorderColor, Color::fromRgba8(100, 180, 255, 255))
    GUT_PROPERTY(f32, borderWidth, 1.0f)
    GUT_PROPERTY(f32, cornerRadius, 4.0f)
    
    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------
    
    void setOnClick(std::function<void()> callback) { m_onClick = std::move(callback); }

protected:
    Size2f measureOverride(Size2f availableSize) override;
    void onRender(RenderContext& ctx) override;
    bool onMouseEvent(const MouseEvent& event) override;
    void onMouseEnter() override;
    void onMouseLeave() override;

private:
    Ref<Element> m_content;
    std::function<void()> m_onClick;
};

} // namespace gut


// --- gut/elements/CheckBox.h ---

#include <functional>

namespace gut {

/**
 * @brief CheckBox control — togglable check with a text label.
 */
class GUT_API CheckBox : public Element {
    GUT_OBJECT(CheckBox, Element)
    
public:
    CheckBox() { setfocusable(true); }
    explicit CheckBox(String label);
    ~CheckBox() override = default;
    
    // -------------------------------------------------------------------------
    // State
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(bool, isChecked, false)
    
    // -------------------------------------------------------------------------
    // Content
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(String, label, "")
    
    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(f32, boxSize, 16.0f)
    GUT_PROPERTY(f32, fontSize, 13.0f)
    GUT_PROPERTY(f32, spacing, 6.0f)
    GUT_PROPERTY(Color, foreground, Color::fromHex(0xE0E0E0))
    GUT_PROPERTY(Color, boxBackground, Color::fromHex(0x3C3C4A))
    GUT_PROPERTY(Color, boxBorderColor, Color::fromHex(0x606070))
    GUT_PROPERTY(Color, checkedBackground, Color::fromRgba8(60, 130, 220, 255))
    GUT_PROPERTY(Color, checkedBorderColor, Color::fromRgba8(80, 150, 240, 255))
    GUT_PROPERTY(Color, checkmarkColor, Color::white())
    GUT_PROPERTY(Color, hoverBorderColor, Color::fromRgba8(100, 160, 240, 255))
    GUT_PROPERTY(Color, disabledForeground, Color::fromHex(0x808080))
    GUT_PROPERTY(f32, boxCornerRadius, 3.0f)
    GUT_PROPERTY(Color, focusBorderColor, Color::fromRgba8(100, 180, 255, 255))
    
    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------
    
    void setOnCheckedChanged(std::function<void(bool)> callback) { m_onCheckedChanged = std::move(callback); }

protected:
    Size2f measureOverride(Size2f availableSize) override;
    void onRender(RenderContext& ctx) override;
    bool onMouseEvent(const MouseEvent& event) override;
    bool onKeyEvent(const KeyEvent& event) override;
    void onMouseEnter() override;
    void onMouseLeave() override;

private:
    void toggle();
    std::function<void(bool)> m_onCheckedChanged;
};

} // namespace gut


// --- gut/elements/RadioButton.h ---

#include <functional>

namespace gut {

/**
 * @brief RadioButton control — mutually exclusive selection within a group.
 *
 * RadioButtons with the same groupName auto-uncheck siblings when one is checked.
 */
class GUT_API RadioButton : public Element {
    GUT_OBJECT(RadioButton, Element)
    
public:
    RadioButton() { setfocusable(true); }
    explicit RadioButton(String label, String group = "default");
    ~RadioButton() override = default;
    
    // -------------------------------------------------------------------------
    // State
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(bool, isChecked, false)
    GUT_PROPERTY(String, groupName, "default")
    
    // -------------------------------------------------------------------------
    // Content
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(String, label, "")
    
    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(f32, circleSize, 16.0f)
    GUT_PROPERTY(f32, fontSize, 13.0f)
    GUT_PROPERTY(f32, spacing, 6.0f)
    GUT_PROPERTY(Color, foreground, Color::fromHex(0xE0E0E0))
    GUT_PROPERTY(Color, circleBackground, Color::fromHex(0x3C3C4A))
    GUT_PROPERTY(Color, circleBorderColor, Color::fromHex(0x606070))
    GUT_PROPERTY(Color, checkedBackground, Color::fromRgba8(60, 130, 220, 255))
    GUT_PROPERTY(Color, checkedBorderColor, Color::fromRgba8(80, 150, 240, 255))
    GUT_PROPERTY(Color, dotColor, Color::white())
    GUT_PROPERTY(Color, hoverBorderColor, Color::fromRgba8(100, 160, 240, 255))
    GUT_PROPERTY(Color, disabledForeground, Color::fromHex(0x808080))
    GUT_PROPERTY(Color, focusBorderColor, Color::fromRgba8(100, 180, 255, 255))
    
    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------
    
    void setOnCheckedChanged(std::function<void(bool)> callback) { m_onCheckedChanged = std::move(callback); }

protected:
    Size2f measureOverride(Size2f availableSize) override;
    void onRender(RenderContext& ctx) override;
    bool onMouseEvent(const MouseEvent& event) override;
    bool onKeyEvent(const KeyEvent& event) override;
    void onMouseEnter() override;
    void onMouseLeave() override;

private:
    void check();
    void uncheckSiblings();
    std::function<void(bool)> m_onCheckedChanged;
};

} // namespace gut


// --- gut/elements/Toggle.h ---

#include <functional>

namespace gut {

/**
 * @brief A toggle switch control — an on/off pill-shaped slider.
 *
 * Similar to CheckBox semantically but rendered as a sliding switch.
 * The thumb slides left (off) / right (on) with an animated transition.
 *
 * Usage:
 *     auto sw = make<Toggle>("Dark mode");
 *     sw->setisOn(true);
 *     sw->setOnToggled([](bool on) { ... });
 */
class GUT_API Toggle : public Element {
    GUT_OBJECT(Toggle, Element)

public:
    Toggle() { setfocusable(true); }
    explicit Toggle(String label);
    ~Toggle() override = default;

    // -------------------------------------------------------------------------
    // State
    // -------------------------------------------------------------------------

    GUT_PROPERTY(bool, isOn, false)

    // -------------------------------------------------------------------------
    // Content
    // -------------------------------------------------------------------------

    GUT_PROPERTY(String, label, "")

    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------

    GUT_PROPERTY(f32, trackWidth, 40.0f)
    GUT_PROPERTY(f32, trackHeight, 22.0f)
    GUT_PROPERTY(f32, thumbInset, 2.0f)       // gap between thumb edge and track edge
    GUT_PROPERTY(f32, fontSize, 13.0f)
    GUT_PROPERTY(f32, spacing, 8.0f)           // gap between track and label

    // Track colors
    GUT_PROPERTY(Color, trackOffBackground, Color::fromHex(0x3C3C4A))
    GUT_PROPERTY(Color, trackOnBackground, Color::fromRgba8(60, 180, 80, 255))
    GUT_PROPERTY(Color, trackOffBorderColor, Color::fromHex(0x606070))
    GUT_PROPERTY(Color, trackOnBorderColor, Color::fromRgba8(50, 160, 65, 255))
    GUT_PROPERTY(Color, hoverBorderColor, Color::fromRgba8(100, 160, 240, 255))
    GUT_PROPERTY(Color, focusBorderColor, Color::fromRgba8(100, 180, 255, 255))

    // Thumb colors
    GUT_PROPERTY(Color, thumbColor, Color::white())
    GUT_PROPERTY(Color, thumbShadowColor, Color::fromRgba8(0, 0, 0, 40))

    // Label
    GUT_PROPERTY(Color, foreground, Color::fromHex(0xE0E0E0))
    GUT_PROPERTY(Color, disabledForeground, Color::fromHex(0x808080))

    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------

    void setOnToggled(std::function<void(bool)> callback) { m_onToggled = std::move(callback); }

protected:
    Size2f measureOverride(Size2f availableSize) override;
    void onRender(RenderContext& ctx) override;
    bool onMouseEvent(const MouseEvent& event) override;
    bool onKeyEvent(const KeyEvent& event) override;
    void onMouseEnter() override;
    void onMouseLeave() override;

private:
    void doToggle();
    f32 thumbRadius() const { return (trackHeight() - thumbInset() * 2.0f) * 0.5f; }
    f32 thumbCenterY() const { return trackHeight() * 0.5f; }
    f32 thumbOffX() const { return thumbInset() + thumbRadius(); }
    f32 thumbOnX() const { return trackWidth() - thumbInset() - thumbRadius(); }

    f32 m_thumbT{0.0f};           // 0 = off, 1 = on (for animation)
    std::function<void(bool)> m_onToggled;
};

} // namespace gut


// --- gut/elements/Slider.h ---

namespace gut {

/**
 * @brief A horizontal slider control for selecting a numeric value within a range.
 *
 * Supports continuous and stepped values, mouse drag, keyboard arrows,
 * and optional label display.
 */
class Slider : public Element {
    GUT_OBJECT(Slider, Element)

public:
    explicit Slider(f32 initialValue = 0.0f);

    // -------------------------------------------------------------------------
    // Value
    // -------------------------------------------------------------------------

    GUT_PROPERTY(f32, value, 0.0f)
    GUT_PROPERTY(f32, minimum, 0.0f)
    GUT_PROPERTY(f32, maximum, 1.0f)
    GUT_PROPERTY(f32, step, 0.0f)              // 0 = continuous

    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------

    GUT_PROPERTY(f32, trackHeight, 4.0f)
    GUT_PROPERTY(f32, thumbRadius, 8.0f)
    GUT_PROPERTY(f32, preferredWidth, 200.0f)
    GUT_PROPERTY(f32, fontSize, 12.0f)
    GUT_PROPERTY(f32, labelSpacing, 8.0f)      // gap between slider and value label

    // Track colors
    GUT_PROPERTY(Color, trackBackground, Color::fromHex(0x3C3C4A))
    GUT_PROPERTY(Color, trackFillColor, Color::fromRgba8(100, 160, 240, 255))
    GUT_PROPERTY(Color, trackBorderColor, Color::fromHex(0x606070))

    // Thumb colors
    GUT_PROPERTY(Color, thumbColor, Color::white())
    GUT_PROPERTY(Color, thumbHoverColor, Color::fromRgba8(220, 230, 255, 255))
    GUT_PROPERTY(Color, thumbPressedColor, Color::fromRgba8(180, 200, 240, 255))
    GUT_PROPERTY(Color, thumbBorderColor, Color::fromRgba8(150, 150, 170, 255))
    GUT_PROPERTY(Color, thumbShadowColor, Color::fromRgba8(0, 0, 0, 40))

    // Focus
    GUT_PROPERTY(Color, focusBorderColor, Color::fromRgba8(100, 180, 255, 255))

    // Disabled
    GUT_PROPERTY(Color, disabledTrackFill, Color::fromHex(0x505060))
    GUT_PROPERTY(Color, disabledThumbColor, Color::fromHex(0x909090))

    // Show numeric value beside the slider
    GUT_PROPERTY(bool, showValue, false)
    GUT_PROPERTY(String, valueFormat, "{:.0f}") // ignored; we just display int-ish

    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------

    void setOnValueChanged(std::function<void(f32)> callback) { m_onValueChanged = std::move(callback); }

protected:
    Size2f measureOverride(Size2f availableSize) override;
    void onRender(RenderContext& ctx) override;
    bool onMouseEvent(const MouseEvent& event) override;
    bool onKeyEvent(const KeyEvent& event) override;
    void onMouseEnter() override;
    void onMouseLeave() override;

private:
    void setValueFromPosition(f32 localX);
    f32 normalizedValue() const;
    f32 trackLeft() const { return thumbRadius(); }
    f32 trackRight() const { return bounds().width - thumbRadius() - valueLabelWidth(); }
    f32 trackWidth() const { return trackRight() - trackLeft(); }
    f32 valueLabelWidth() const;

    bool m_dragging{false};
    std::function<void(f32)> m_onValueChanged;
};

} // namespace gut


// --- gut/elements/ProgressBar.h ---

namespace gut {

/**
 * @brief A progress bar control that displays a completion percentage.
 *
 * Supports determinate (0–100%) and indeterminate (animated pulse) modes,
 * optional text label, and customisable colours.
 */
class ProgressBar : public Element {
    GUT_OBJECT(ProgressBar, Element)

public:
    explicit ProgressBar(f32 initialValue = 0.0f);

    // -------------------------------------------------------------------------
    // Value
    // -------------------------------------------------------------------------

    GUT_PROPERTY(f32, value, 0.0f)        // 0 – 100
    GUT_PROPERTY(f32, minimum, 0.0f)
    GUT_PROPERTY(f32, maximum, 100.0f)
    GUT_PROPERTY(bool, indeterminate, false)  // animated pulse mode

    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------

    GUT_PROPERTY(f32, barHeight, 8.0f)
    GUT_PROPERTY(f32, preferredWidth, 250.0f)
    GUT_PROPERTY(f32, cornerRadius, 4.0f)
    GUT_PROPERTY(f32, fontSize, 10.0f)

    // Colours
    GUT_PROPERTY(Color, trackColor, Color::fromHex(0x3C3C4A))
    GUT_PROPERTY(Color, trackBorderColor, Color::fromHex(0x505060))
    GUT_PROPERTY(Color, fillColor, Color::fromRgba8(80, 150, 240, 255))
    GUT_PROPERTY(Color, completedFillColor, Color::fromRgba8(60, 190, 80, 255))
    GUT_PROPERTY(Color, labelColor, Color::fromHex(0xC0C0D0))

    // Show percentage text
    GUT_PROPERTY(bool, showLabel, false)
    GUT_PROPERTY(bool, showPercentInBar, false)  // draw % centred inside bar

protected:
    Size2f measureOverride(Size2f availableSize) override;
    void onRender(RenderContext& ctx) override;

private:
    f32 normalizedValue() const;
};

} // namespace gut


// --- gut/elements/DropDown.h ---



namespace gut {

/**
 * @brief A dropdown menu control that displays a list of selectable items.
 *
 * Features:
 *   - Single-selection dropdown (not a combo box — no text editing)
 *   - Scrollable item list when items exceed maxDropHeight
 *   - Automatically opens upward or downward based on available screen space
 *   - Keyboard navigation: Up/Down to highlight, Enter/Space to select, Escape to close
 *   - Click outside to dismiss
 */
class GUT_API DropDown : public Element {
    GUT_OBJECT(DropDown, Element)
public:
    DropDown();
    ~DropDown() override = default;

    // -------------------------------------------------------------------------
    // Items
    // -------------------------------------------------------------------------
    void addItem(String item);
    void removeItem(usize index);
    void clearItems();
    usize itemCount() const { return m_items.size(); }
    const String& itemAt(usize index) const { return m_items[index]; }

    // -------------------------------------------------------------------------
    // Selection
    // -------------------------------------------------------------------------
    GUT_PROPERTY(isize, selectedIndex, -1)
    String selectedItem() const;

    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------
    GUT_PROPERTY(f32, fontSize, 13.0f)
    GUT_PROPERTY(f32, itemHeight, 26.0f)
    GUT_PROPERTY(f32, maxDropHeight, 200.0f)
    GUT_PROPERTY(f32, cornerRadius, 4.0f)
    GUT_PROPERTY(f32, arrowSize, 8.0f)
    GUT_PROPERTY(String, placeholder, "Select...")

    // Colors — button
    GUT_PROPERTY(Color, foreground, Color::fromHex(0xE0E0E0))
    GUT_PROPERTY(Color, buttonBackground, Color::fromHex(0x2D2D3A))
    GUT_PROPERTY(Color, borderColor, Color::fromHex(0x606070))
    GUT_PROPERTY(Color, hoverBorderColor, Color::fromRgba8(100, 160, 240, 255))
    GUT_PROPERTY(Color, focusBorderColor, Color::fromRgba8(100, 180, 255, 255))
    GUT_PROPERTY(Color, arrowColor, Color::fromHex(0xA0A0B0))

    // Colors — popup
    GUT_PROPERTY(Color, dropBackground, Color::fromHex(0x252534))
    GUT_PROPERTY(Color, dropBorderColor, Color::fromHex(0x505060))
    GUT_PROPERTY(Color, itemHoverBackground, Color::fromRgba8(60, 60, 80, 255))
    GUT_PROPERTY(Color, selectedItemBackground, Color::fromRgba8(60, 130, 220, 255))
    GUT_PROPERTY(Color, scrollbarTrackColor, Color::fromRgba8(40, 40, 55, 80))
    GUT_PROPERTY(Color, scrollbarThumbColor, Color::fromRgba8(100, 100, 120, 160))

    GUT_PROPERTY(Color, disabledForeground, Color::fromHex(0x808080))

    // -------------------------------------------------------------------------
    // State
    // -------------------------------------------------------------------------
    bool isOpen() const { return m_isOpen; }
    void open();
    void close();
    void toggle();

    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------
    void setOnSelectionChanged(std::function<void(isize)> callback) { m_onSelectionChanged = std::move(callback); }

protected:
    Size2f measureOverride(Size2f availableSize) override;
    void onRender(RenderContext& ctx) override;
    bool onMouseEvent(const MouseEvent& event) override;
    bool onKeyEvent(const KeyEvent& event) override;
    void onMouseEnter() override;
    void onMouseLeave() override;

private:
    std::vector<String> m_items;
    bool m_isOpen{false};
    bool m_opensUpward{false};
    f32 m_scrollOffset{0.0f};
    isize m_hoveredItemIndex{-1};
    f32 m_popupHeight{0.0f};
    std::function<void(isize)> m_onSelectionChanged;

    // Helpers
    f32 totalItemsHeight() const;
    f32 effectivePopupHeight() const;
    bool needsScroll() const;
    f32 maxScrollOffset() const;
    Rectf popupLocalRect() const;
    isize itemIndexAtLocalY(f32 localY) const;
    void renderPopupOverlay(RenderContext& ctx);
    static constexpr f32 kScrollbarWidth = 8.0f;
    static constexpr f32 kPadH = 8.0f;  // horizontal padding inside button
};

} // namespace gut


// --- gut/elements/TabControl.h ---



namespace gut {

/**
 * @brief A tab control that displays tabbed pages.
 *
 * Each tab has a header label and a content element. Only the selected tab's
 * content is visible. The tab bar is rendered at the top; the content area
 * fills the remainder.
 *
 * Usage:
 *     auto tabs = make<TabControl>();
 *     tabs->addTab("General", generalPanel);
 *     tabs->addTab("Advanced", advancedPanel);
 *     tabs->setselectedIndex(0);
 */
class GUT_API TabControl : public Panel {
    GUT_OBJECT(TabControl, Panel)
public:
    TabControl();
    ~TabControl() override = default;

    // -------------------------------------------------------------------------
    // Tabs
    // -------------------------------------------------------------------------
    void addTab(String title, Ref<Element> content);
    void removeTab(usize index);
    usize tabCount() const { return m_tabs.size(); }

    GUT_PROPERTY(isize, selectedIndex, 0)
    void selectTab(isize index);

    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------
    GUT_PROPERTY(f32, tabBarHeight, 32.0f)
    GUT_PROPERTY(f32, tabFontSize, 12.0f)
    GUT_PROPERTY(f32, tabPadding, 16.0f)    // horizontal padding per tab
    GUT_PROPERTY(f32, tabSpacing, 2.0f)     // gap between tabs

    // Colors — tab bar
    GUT_PROPERTY(Color, tabBarBackground, Color::fromHex(0x252534))
    GUT_PROPERTY(Color, tabBarBorderColor, Color::fromHex(0x404050))

    // Colors — tab headers
    GUT_PROPERTY(Color, tabForeground, Color::fromHex(0xA0A0B0))
    GUT_PROPERTY(Color, tabActiveForeground, Color::fromHex(0xE0E0F0))
    GUT_PROPERTY(Color, tabActiveIndicator, Color::fromRgba8(80, 150, 240, 255))
    GUT_PROPERTY(Color, tabHoverBackground, Color::fromRgba8(60, 60, 80, 100))

    // Colors — content area
    GUT_PROPERTY(Color, contentBackground, Color::transparent())

    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------
    void setOnTabChanged(std::function<void(isize)> callback) { m_onTabChanged = std::move(callback); }

protected:
    Size2f measureOverride(Size2f availableSize) override;
    Size2f arrangeOverride(Size2f finalSize) override;
    void onRender(RenderContext& ctx) override;
    bool onMouseEvent(const MouseEvent& event) override;
    bool onKeyEvent(const KeyEvent& event) override;

private:
    struct Tab {
        String title;
        Ref<Element> content;
    };
    std::vector<Tab> m_tabs;
    isize m_hoveredTab{-1};
    std::function<void(isize)> m_onTabChanged;

    // Scroll state (when tabs overflow)
    f32 m_tabScrollOffset{0.0f};
    bool m_leftArrowHovered{false};
    bool m_rightArrowHovered{false};

    // Helpers
    isize tabIndexAtX(f32 x) const;
    f32 tabHeaderWidth(const String& title) const;
    f32 totalTabsWidth() const;
    bool needsScroll() const;
    f32 scrollArrowWidth() const { return 28.0f; }
    f32 scrollableRegionWidth() const;
    void ensureTabVisible(isize index);
};

} // namespace gut


// --- gut/elements/Table.h ---



namespace gut {

/**
 * @brief A sortable, scrollable data table.
 *
 * The Table displays rows and columns of text data with clickable column
 * headers for sorting.  Sorting is NOT done internally — instead, a callback
 * fires when a column header is clicked, letting the application re-order the
 * data and call setRows() with the new list.
 *
 * Features:
 *   - Scrollable body (mouse wheel) for large row counts
 *   - Column-header click → onSortRequested(columnIndex, ascending)
 *   - Row selection → onRowSelected(rowIndex)
 *   - Row hover highlighting
 *   - Sort indicator arrow on the active sort column
 *   - Styled header bar, alternating row colors, selection highlight
 *
 * Usage:
 *     auto table = make<Table>();
 *     table->addColumn("Name", 200);
 *     table->addColumn("Ping", 80);
 *     table->addColumn("Players", 100);
 *     table->setRows({ {"Realm 1", "32 ms", "1024"}, ... });
 *     table->setOnSortRequested([](usize col, bool asc) { ... });
 *     table->setOnRowSelected([](isize row) { ... });
 */
class GUT_API Table : public Element {
    GUT_OBJECT(Table, Element)

public:
    Table();
    ~Table() override = default;

    // -------------------------------------------------------------------------
    // Columns
    // -------------------------------------------------------------------------

    struct Column {
        String title;
        f32 width{120.0f};
    };

    void addColumn(String title, f32 width = 120.0f);
    void clearColumns();
    usize columnCount() const { return m_columns.size(); }
    const Column& columnAt(usize index) const { return m_columns[index]; }

    // -------------------------------------------------------------------------
    // Data  (each row is a vector of strings, one per column)
    // -------------------------------------------------------------------------

    using Row = std::vector<String>;
    void setRows(std::vector<Row> rows);
    void clearRows();
    usize rowCount() const { return m_rows.size(); }
    const Row& rowAt(usize index) const { return m_rows[index]; }

    // -------------------------------------------------------------------------
    // Selection
    // -------------------------------------------------------------------------

    GUT_PROPERTY(isize, selectedRow, -1)
    void selectRow(isize index);

    // -------------------------------------------------------------------------
    // Sort state (visual only — the app provides sorted data)
    // -------------------------------------------------------------------------

    GUT_PROPERTY(isize, sortColumn, -1)
    GUT_PROPERTY(bool, sortAscending, true)

    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------

    GUT_PROPERTY(f32, rowHeight, 28.0f)
    GUT_PROPERTY(f32, headerHeight, 30.0f)
    GUT_PROPERTY(f32, fontSize, 12.0f)
    GUT_PROPERTY(f32, headerFontSize, 12.0f)
    GUT_PROPERTY(f32, cellPaddingH, 10.0f)

    // Colors — header
    GUT_PROPERTY(Color, headerBackground, Color::fromHex(0x2A2A3A))
    GUT_PROPERTY(Color, headerForeground, Color::fromHex(0xC0C0D8))
    GUT_PROPERTY(Color, headerBorderColor, Color::fromHex(0x404058))
    GUT_PROPERTY(Color, headerHoverBackground, Color::fromRgba8(55, 55, 75, 255))
    GUT_PROPERTY(Color, sortArrowColor, Color::fromRgba8(120, 180, 255, 255))

    // Colors — body
    GUT_PROPERTY(Color, rowBackground, Color::fromHex(0x1E1E2A))
    GUT_PROPERTY(Color, rowAlternateBackground, Color::fromHex(0x232332))
    GUT_PROPERTY(Color, rowHoverBackground, Color::fromRgba8(50, 50, 68, 255))
    GUT_PROPERTY(Color, rowSelectedBackground, Color::fromRgba8(50, 100, 200, 255))
    GUT_PROPERTY(Color, cellForeground, Color::fromHex(0xD0D0E0))
    GUT_PROPERTY(Color, gridLineColor, Color::fromRgba8(50, 50, 62, 100))

    // Colors — scrollbar
    GUT_PROPERTY(Color, scrollbarTrackColor, Color::fromRgba8(40, 40, 55, 80))
    GUT_PROPERTY(Color, scrollbarThumbColor, Color::fromRgba8(100, 100, 120, 160))

    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------

    /** Called when a column header is clicked.  App should sort and call setRows(). */
    void setOnSortRequested(std::function<void(usize columnIndex, bool ascending)> cb) {
        m_onSortRequested = std::move(cb);
    }

    /** Called when a row is selected (clicked). -1 if deselected. */
    void setOnRowSelected(std::function<void(isize rowIndex)> cb) {
        m_onRowSelected = std::move(cb);
    }

protected:
    Size2f measureOverride(Size2f availableSize) override;
    void onRender(RenderContext& ctx) override;
    bool onMouseEvent(const MouseEvent& event) override;
    bool onKeyEvent(const KeyEvent& event) override;
    void onMouseEnter() override;
    void onMouseLeave() override;

private:
    f32 totalColumnsWidth() const;
    f32 totalRowsHeight() const { return static_cast<f32>(m_rows.size()) * rowHeight(); }
    f32 bodyHeight() const { return bounds().height - headerHeight(); }
    f32 maxScrollOffset() const { return std::max(0.0f, totalRowsHeight() - bodyHeight()); }
    bool needsScrollbar() const { return totalRowsHeight() > bodyHeight(); }
    isize rowIndexAtY(f32 localY) const;       // -1 if in header or out of range
    isize columnIndexAtX(f32 localX) const;

    static constexpr f32 kScrollbarWidth = 8.0f;
    static constexpr f32 kSortArrowSize  = 6.0f;

    std::vector<Column> m_columns;
    std::vector<Row> m_rows;

    f32 m_scrollOffset{0.0f};
    isize m_hoveredRow{-1};
    isize m_hoveredHeaderCol{-1};

    // Scrollbar thumb drag
    bool m_draggingThumb{false};
    f32 m_dragStartY{0.0f};
    f32 m_dragStartScroll{0.0f};

    Rectf scrollbarThumbRect() const;

    std::function<void(usize, bool)> m_onSortRequested;
    std::function<void(isize)> m_onRowSelected;
};

} // namespace gut


// --- gut/elements/ListView.h ---

#include <functional>
#include <set>

namespace gut {

/**
 * @brief Selection behaviour for ListView.
 */
enum class SelectionMode : u8 {
    None,       ///< No selection allowed
    Single,     ///< Exactly one item at a time
    Multiple,   ///< Ctrl+click to toggle individual items
    Extended    ///< Shift+click ranges + Ctrl+click toggle (like a file manager)
};

/**
 * @brief A virtualised, scrollable list of items.
 *
 * Provide an item count and a factory function that builds an element for a
 * given index.  ListView renders only the visible rows, supports keyboard
 * navigation (Up / Down / Home / End / PageUp / PageDown) and several
 * selection modes.
 *
 * Usage:
 *     auto list = make<ListView>();
 *     list->setItemCount(1000);
 *     list->setItemTemplate([](isize index) {
 *         auto t = make<Text>("Item #" + std::to_string(index), 12.0f);
 *         t->setforeground(Color::white());
 *         return Ref<Element>(t);
 *     });
 *     list->setOnSelectionChanged([](const std::set<isize>& sel) { ... });
 */
class GUT_API ListView : public Element {
    GUT_OBJECT(ListView, Element)

public:
    ListView();
    ~ListView() override = default;

    // -------------------------------------------------------------------------
    // Data
    // -------------------------------------------------------------------------

    /// Set the total number of items (re-measures; clears selection).
    void setItemCount(isize count);
    isize itemCount() const { return m_itemCount; }

    /// Factory called for each visible row.  Receives the item index, returns
    /// a freshly-configured element (or a recycled one — caller's choice).
    using ItemTemplate = std::function<Ref<Element>(isize index)>;
    void setItemTemplate(ItemTemplate tmpl) { m_itemTemplate = std::move(tmpl); }

    /// Force a refresh of all visible items (call after mutating data).
    void refresh();

    // -------------------------------------------------------------------------
    // Selection
    // -------------------------------------------------------------------------

    GUT_PROPERTY(SelectionMode, selectionMode, SelectionMode::Single)

    /// Currently selected indices (may be >1 in Multiple / Extended mode).
    const std::set<isize>& selectedIndices() const { return m_selectedIndices; }

    /// Convenience: first selected index (-1 when empty).
    isize selectedIndex() const;

    /// Programmatic selection.
    void selectIndex(isize index);
    void deselectAll();
    void selectRange(isize from, isize to);   // inclusive

    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------

    GUT_PROPERTY(f32, itemHeight, 28.0f)
    GUT_PROPERTY(f32, fontSize, 12.0f)        // only used if no template

    // Colours — body
    GUT_PROPERTY(Color, listBackground, Color::fromHex(0x1E1E2A))
    GUT_PROPERTY(Color, itemBackground, Color::transparent())
    GUT_PROPERTY(Color, itemAlternateBackground, Color::fromRgba8(35, 35, 50, 255))
    GUT_PROPERTY(Color, itemHoverBackground, Color::fromRgba8(50, 50, 68, 255))
    GUT_PROPERTY(Color, itemSelectedBackground, Color::fromRgba8(50, 100, 200, 255))
    GUT_PROPERTY(Color, itemForeground, Color::fromHex(0xD0D0E0))
    GUT_PROPERTY(Color, dividerColor, Color::fromRgba8(50, 50, 62, 100))

    // Colours — scrollbar
    GUT_PROPERTY(Color, scrollbarTrackColor, Color::fromRgba8(40, 40, 55, 80))
    GUT_PROPERTY(Color, scrollbarThumbColor, Color::fromRgba8(100, 100, 120, 160))

    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------

    /// Fired after any selection change.
    void setOnSelectionChanged(std::function<void(const std::set<isize>&)> cb) {
        m_onSelectionChanged = std::move(cb);
    }

    /// Fired on double-click of an item.
    void setOnItemDoubleClicked(std::function<void(isize)> cb) {
        m_onItemDoubleClicked = std::move(cb);
    }

protected:
    Size2f measureOverride(Size2f availableSize) override;
    void onRender(RenderContext& ctx) override;
    bool onMouseEvent(const MouseEvent& event) override;
    bool onKeyEvent(const KeyEvent& event) override;
    void onMouseEnter() override;
    void onMouseLeave() override;

private:
    // Geometry helpers
    f32 totalHeight() const { return static_cast<f32>(m_itemCount) * itemHeight(); }
    f32 viewHeight() const { return bounds().height; }
    f32 maxScrollOffset() const { return std::max(0.0f, totalHeight() - viewHeight()); }
    bool needsScrollbar() const { return totalHeight() > viewHeight(); }
    isize itemIndexAtY(f32 localY) const;

    static constexpr f32 kScrollbarWidth = 8.0f;
    Rectf scrollbarThumbRect() const;

    void ensureVisible(isize index);
    void notifySelectionChanged();

    // Data
    isize m_itemCount{0};
    ItemTemplate m_itemTemplate;

    // Selection
    std::set<isize> m_selectedIndices;
    isize m_anchorIndex{-1};   // anchor for shift-click ranges

    // Scroll
    f32 m_scrollOffset{0.0f};
    isize m_hoveredItem{-1};

    // Scrollbar drag state
    bool m_draggingThumb{false};
    f32 m_dragStartY{0.0f};
    f32 m_dragStartScroll{0.0f};

    std::function<void(const std::set<isize>&)> m_onSelectionChanged;
    std::function<void(isize)> m_onItemDoubleClicked;
};

} // namespace gut


// --- gut/elements/Dialog.h ---

#include <functional>

namespace gut {

/**
 * @brief Result from a dialog interaction.
 */
enum class DialogResult {
    None,
    OK,
    Cancel,
    Yes,
    No
};

/**
 * @brief Predefined button sets for common dialog patterns.
 */
enum class DialogButtons {
    None,           // No buttons — user must close via X or Escape
    OK,             // [OK]
    OKCancel,       // [OK] [Cancel]
    YesNo,          // [Yes] [No]
    YesNoCancel     // [Yes] [No] [Cancel]
};

/**
 * @brief A modal dialog overlay control.
 *
 * Features:
 *   - Semi-transparent backdrop blocks interaction with the rest of the UI
 *   - Title bar with close (X) button, draggable to reposition
 *   - Message text and/or custom content element
 *   - Configurable button sets (OK, OK/Cancel, Yes/No, etc.)
 *   - Keyboard: Escape to close, Enter/Return to accept default button
 *   - Smooth appearance with drop shadow
 *   - Click outside (backdrop) to dismiss (optional)
 */
class GUT_API Dialog : public Element {
    GUT_OBJECT(Dialog, Element)
public:
    Dialog();
    ~Dialog() override = default;

    // -------------------------------------------------------------------------
    // Content
    // -------------------------------------------------------------------------

    GUT_PROPERTY(String, title, "Dialog")
    GUT_PROPERTY(String, message, "")

    /**
     * @brief Set a custom content element for the dialog body.
     * If set, this replaces the message text.
     */
    void setContent(Ref<Element> content);
    Element* content() const { return m_content.get(); }

    // -------------------------------------------------------------------------
    // Buttons
    // -------------------------------------------------------------------------

    GUT_PROPERTY(DialogButtons, buttons, DialogButtons::OK)

    // -------------------------------------------------------------------------
    // Sizing
    // -------------------------------------------------------------------------

    GUT_PROPERTY(f32, dialogWidth, 420.0f)
    GUT_PROPERTY(f32, dialogHeight, 200.0f)

    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------

    GUT_PROPERTY(f32, cornerRadius, 8.0f)
    GUT_PROPERTY(f32, titleFontSize, 15.0f)
    GUT_PROPERTY(f32, messageFontSize, 13.0f)
    GUT_PROPERTY(f32, buttonFontSize, 13.0f)
    GUT_PROPERTY(f32, titleBarHeight, 40.0f)
    GUT_PROPERTY(f32, buttonHeight, 32.0f)
    GUT_PROPERTY(f32, buttonWidth, 80.0f)
    GUT_PROPERTY(f32, buttonSpacing, 10.0f)
    GUT_PROPERTY(f32, buttonAreaHeight, 52.0f)

    // Colors — backdrop
    GUT_PROPERTY(Color, backdropColor, Color::fromRgba8(0, 0, 0, 120))

    // Colors — dialog chrome
    GUT_PROPERTY(Color, dialogBackground, Color::fromHex(0x2A2A3A))
    GUT_PROPERTY(Color, dialogBorderColor, Color::fromHex(0x505068))
    GUT_PROPERTY(Color, titleBarBackground, Color::fromHex(0x323248))
    GUT_PROPERTY(Color, titleForeground, Color::fromHex(0xE0E0F0))
    GUT_PROPERTY(Color, closeButtonColor, Color::fromHex(0x808098))
    GUT_PROPERTY(Color, closeButtonHoverColor, Color::fromRgba8(240, 80, 80, 255))

    // Colors — body
    GUT_PROPERTY(Color, messageForeground, Color::fromHex(0xC8C8DC))

    // Colors — buttons
    GUT_PROPERTY(Color, primaryButtonBackground, Color::fromRgba8(60, 130, 220, 255))
    GUT_PROPERTY(Color, primaryButtonHoverBackground, Color::fromRgba8(80, 150, 240, 255))
    GUT_PROPERTY(Color, primaryButtonForeground, Color::white())
    GUT_PROPERTY(Color, secondaryButtonBackground, Color::fromHex(0x3A3A4E))
    GUT_PROPERTY(Color, secondaryButtonHoverBackground, Color::fromHex(0x4A4A60))
    GUT_PROPERTY(Color, secondaryButtonForeground, Color::fromHex(0xD0D0E0))
    GUT_PROPERTY(Color, buttonBorderColor, Color::fromHex(0x505068))

    // -------------------------------------------------------------------------
    // Behavior
    // -------------------------------------------------------------------------

    GUT_PROPERTY(bool, dismissOnBackdropClick, false)

    // -------------------------------------------------------------------------
    // Show / Close
    // -------------------------------------------------------------------------

    void show();
    void close(DialogResult result = DialogResult::Cancel);
    bool isVisible() const { return m_visible; }

    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------

    void setOnResult(std::function<void(DialogResult)> cb) { m_onResult = std::move(cb); }
    void setOnClosed(std::function<void()> cb) { m_onClosed = std::move(cb); }

protected:
    Size2f measureOverride(Size2f availableSize) override;
    void onRender(RenderContext& ctx) override;
    bool onMouseEvent(const MouseEvent& event) override;
    bool onKeyEvent(const KeyEvent& event) override;

private:
    bool m_visible{false};
    Ref<Element> m_content;
    std::function<void(DialogResult)> m_onResult;
    std::function<void()> m_onClosed;

    // Drag state (title bar)
    bool m_dragging{false};
    f32 m_dragStartX{0.0f};
    f32 m_dragStartY{0.0f};
    f32 m_dialogOffsetX{0.0f};  // offset from default centered position
    f32 m_dialogOffsetY{0.0f};
    f32 m_dragStartOffsetX{0.0f};
    f32 m_dragStartOffsetY{0.0f};

    // Hover state for interactive regions
    bool m_closeHovered{false};
    isize m_hoveredButton{-1};

    // Helpers
    Rectf dialogRect() const;        // in screen coords
    Rectf titleBarRect() const;      // relative to dialog
    Rectf closeButtonRect() const;   // relative to dialog
    Rectf bodyRect() const;          // relative to dialog
    Rectf buttonAreaRect() const;    // relative to dialog
    struct ButtonInfo { Rectf rect; String label; bool primary; DialogResult result; };
    std::vector<ButtonInfo> buttonLayout() const;

    void renderOverlay(RenderContext& ctx);

    static constexpr f32 kCloseButtonSize = 16.0f;
    static constexpr f32 kPadding = 20.0f;
};

} // namespace gut


// --- gut/elements/Tooltip.h ---



namespace gut {

/**
 * @brief Lightweight tooltip that appears on hover.
 *
 * Attach a Tooltip to any element via Tooltip::set(element, text).
 * The tooltip appears after a configurable delay and positions itself
 * near the mouse cursor using the overlay system.
 *
 * Usage:
 *     auto btn = make<Button>("Hover me");
 *     Tooltip::set(btn, "This is a helpful tooltip");
 */
class GUT_API Tooltip {
public:
    /// Attach a tooltip to an element. Pass empty text to remove.
    static void set(Ref<Element> element, String text);

    /// Attach with custom delay (ms).
    static void set(Ref<Element> element, String text, f32 delayMs);

    // -------------------------------------------------------------------------
    // Global appearance configuration
    // -------------------------------------------------------------------------
    struct Style {
        Color background{Color::fromRgba8(40, 40, 55, 240)};
        Color foreground{Color::fromRgba8(220, 220, 240, 255)};
        Color borderColor{Color::fromRgba8(80, 80, 100, 200)};
        f32 fontSize{11.0f};
        f32 cornerRadius{4.0f};
        f32 paddingH{8.0f};
        f32 paddingV{5.0f};
        f32 offsetY{20.0f};     // below cursor
        f32 delayMs{500.0f};    // default show delay
        f32 maxWidth{300.0f};   // word-wrap threshold
    };

    static Style& style();

private:
    Tooltip() = default;
};

} // namespace gut


// --- gut/elements/ContextMenu.h ---



namespace gut {

/**
 * @brief A popup context menu that appears on right-click.
 *
 * Define menu items, then attach to elements via ContextMenu::attachTo().
 * The menu appears at the click position as an overlay and dismisses when
 * an item is clicked or the user clicks outside.
 *
 * Usage:
 *     auto menu = make<ContextMenu>();
 *     menu->addItem("Cut",   [&]{ doCut(); });
 *     menu->addItem("Copy",  [&]{ doCopy(); });
 *     menu->addItem("Paste", [&]{ doPaste(); });
 *     menu->addSeparator();
 *     menu->addItem("Delete", [&]{ doDelete(); });
 *     menu->attachTo(somePanel);
 */
class GUT_API ContextMenu : public Element {
    GUT_OBJECT(ContextMenu, Element)

public:
    ContextMenu();
    ~ContextMenu() override = default;

    // -------------------------------------------------------------------------
    // Items
    // -------------------------------------------------------------------------

    struct MenuItem {
        String label;
        std::function<void()> action;
        bool separator{false};
        bool enabled{true};
    };

    void addItem(String label, std::function<void()> action, bool enabled = true);
    void addSeparator();
    void clearItems();
    usize itemCount() const { return m_items.size(); }

    // -------------------------------------------------------------------------
    // Attach / show / hide
    // -------------------------------------------------------------------------

    /// Attach to an element — installs a right-click handler.
    void attachTo(Ref<Element> element);

    /// Show the menu at a specific position (screen coordinates).
    void showAt(f32 x, f32 y);

    /// Programmatically close the menu.
    void close();

    bool isOpen() const { return m_isOpen; }

    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------

    GUT_PROPERTY(f32, fontSize, 12.0f)
    GUT_PROPERTY(f32, itemHeight, 26.0f)
    GUT_PROPERTY(f32, menuWidth, 180.0f)
    GUT_PROPERTY(f32, cornerRadius, 6.0f)
    GUT_PROPERTY(f32, separatorHeight, 1.0f)

    GUT_PROPERTY(Color, menuBackground, Color::fromRgba8(35, 35, 48, 245))
    GUT_PROPERTY(Color, menuBorderColor, Color::fromRgba8(70, 70, 90, 200))
    GUT_PROPERTY(Color, itemForeground, Color::fromRgba8(210, 210, 230, 255))
    GUT_PROPERTY(Color, itemHoverBackground, Color::fromRgba8(60, 100, 200, 255))
    GUT_PROPERTY(Color, itemHoverForeground, Color::fromRgba8(255, 255, 255, 255))
    GUT_PROPERTY(Color, disabledForeground, Color::fromRgba8(100, 100, 120, 255))
    GUT_PROPERTY(Color, separatorColor, Color::fromRgba8(60, 60, 78, 200))

    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------
    void setOnClosed(std::function<void()> cb) { m_onClosed = std::move(cb); }

protected:
    Size2f measureOverride(Size2f availableSize) override;
    void onRender(RenderContext& ctx) override;
    bool onMouseEvent(const MouseEvent& event) override;
    bool onKeyEvent(const KeyEvent& event) override;

private:
    std::vector<MenuItem> m_items;
    bool m_isOpen{false};
    f32 m_popupX{0.0f};
    f32 m_popupY{0.0f};
    isize m_hoveredItemIndex{-1};
    std::function<void()> m_onClosed;

    void renderPopupOverlay(RenderContext& ctx);
    f32 totalMenuHeight() const;
    isize itemIndexAtY(f32 localY) const;
};

} // namespace gut


// --- gut/elements/Toast.h ---

namespace gut {

/**
 * @brief Screen position for Toast notifications.
 */
enum class ToastPosition : u8 {
    TopLeft,
    TopCenter,
    TopRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

/**
 * @brief A lightweight, auto-dismissing notification overlay.
 *
 * Toast notifications appear at a configurable screen position and
 * automatically disappear after a timeout.  Multiple toasts stack
 * vertically.
 *
 * Usage:
 *     Toast::show(context, "Connection lost", ToastPosition::BottomRight);
 *     Toast::show(context, "Saved!", ToastPosition::TopCenter, 2.0f);
 */
class GUT_API Toast {
public:
    /// Show a toast.  durationSec <= 0 means use the global default.
    static void show(Context* ctx, String message,
                     ToastPosition position = ToastPosition::BottomRight,
                     f32 durationSec = 0.0f);

    /// Dismiss all visible toasts immediately.
    static void dismissAll(Context* ctx);

    // -------------------------------------------------------------------------
    // Global appearance configuration
    // -------------------------------------------------------------------------
    struct Style {
        Color background{Color::fromRgba8(40, 40, 55, 230)};
        Color foreground{Color::fromRgba8(220, 220, 240, 255)};
        Color borderColor{Color::fromRgba8(80, 80, 100, 180)};
        f32 fontSize{12.0f};
        f32 cornerRadius{6.0f};
        f32 paddingH{14.0f};
        f32 paddingV{10.0f};
        f32 maxWidth{320.0f};
        f32 margin{12.0f};         // distance from screen edge
        f32 spacing{8.0f};         // gap between stacked toasts
        f32 defaultDuration{3.0f}; // seconds
    };

    static Style& style();

private:
    Toast() = default;
};

} // namespace gut


// --- gut/elements/Icon.h ---

namespace gut {

/**
 * @brief Built-in procedural icon identifiers.
 *
 * Each icon is drawn using RenderContext primitives (lines, rects, ellipses,
 * polygons) so there is no dependency on external assets or icon fonts.
 */
enum class IconName : u8 {
    None = 0,

    // --- File / document ---
    FileNew,        ///< blank page
    FileOpen,       ///< folder
    Save,           ///< floppy disk
    SaveAs,         ///< floppy + pencil

    // --- Edit ---
    Cut,            ///< scissors
    Copy,           ///< two overlapping pages
    Paste,          ///< clipboard
    Undo,           ///< curved arrow left
    Redo,           ///< curved arrow right
    Delete,         ///< trash can

    // --- Text formatting ---
    Bold,           ///< B
    Italic,         ///< I (slanted)
    Underline,      ///< U with underline
    Strikethrough,  ///< S with strikethrough
    AlignLeft,      ///< 3 left-aligned lines
    AlignCenter,    ///< 3 centred lines
    AlignRight,     ///< 3 right-aligned lines

    // --- Navigation ---
    ArrowUp,
    ArrowDown,
    ArrowLeft,
    ArrowRight,
    Home,           ///< house
    Search,         ///< magnifying glass
    Refresh,        ///< circular arrow

    // --- Actions ---
    Plus,           ///< +
    Minus,          ///< −
    Close,          ///< X
    Check,          ///< checkmark
    Settings,       ///< gear / cog
    Menu,           ///< hamburger (3 horizontal lines)
    MoreHorizontal, ///< three dots (...)
    MoreVertical,   ///< three vertical dots

    // --- Media ---
    Play,           ///< right triangle
    Pause,          ///< two bars
    Stop,           ///< solid square
    SkipForward,    ///< >>|
    SkipBack,       ///< |<<

    // --- Misc ---
    Info,           ///< circled i
    Warning,        ///< triangle with !
    Error,          ///< circled X
    Star,           ///< 5-point star
    Heart,          ///< heart shape
    Eye,            ///< eye (visible)
    EyeOff,         ///< eye with line through
    Lock,           ///< padlock
    Unlock,         ///< open padlock
    User,           ///< person silhouette
    Download,       ///< arrow pointing down into tray
    Upload,         ///< arrow pointing up from tray

    _Count
};

/**
 * @brief Static helper to draw built-in procedural icons.
 *
 * Usage:
 *     Icon::draw(ctx, IconName::Save, {10, 10, 20, 20}, Color::white());
 *
 * Icons are drawn within the given bounds rectangle, scaled proportionally.
 * A stroke thickness is derived from the icon size for crisp rendering.
 */
class GUT_API Icon {
public:
    /// Draw an icon into the given bounds.
    static void draw(RenderContext& ctx, IconName name, Rectf bounds, Color color,
                     f32 strokeWeight = 0.0f);

    /// Convenience: draw centred within a square of `size` at position.
    static void draw(RenderContext& ctx, IconName name, Point2f pos, f32 size, Color color,
                     f32 strokeWeight = 0.0f);

private:
    Icon() = default;
};

/**
 * @brief Element that displays a built-in icon.
 *
 * Usage:
 *     auto icon = make<IconElement>(IconName::Save);
 *     icon->setsize(24.0f);
 *     icon->setcolor(Color::white());
 */
class GUT_API IconElement : public Element {
    GUT_OBJECT(IconElement, Element)

public:
    IconElement() = default;
    explicit IconElement(IconName name);
    ~IconElement() override = default;

    GUT_PROPERTY(IconName, icon, IconName::None)
    GUT_PROPERTY(f32, size, 20.0f)
    GUT_PROPERTY(Color, color, Color::white())
    GUT_PROPERTY(f32, strokeWeight, 0.0f)

protected:
    Size2f measureOverride(Size2f availableSize) override;
    void onRender(RenderContext& ctx) override;
};

} // namespace gut


// --- gut/elements/Toolbar.h ---

namespace gut {

/**
 * @brief The type of a toolbar item.
 */
enum class ToolbarItemType : u8 {
    Button,     ///< Clickable button with icon and/or label
    Toggle,     ///< Like Button but has pressed/toggled state
    Separator,  ///< Thin vertical divider
};

/**
 * @brief A horizontal strip of icon buttons, toggles, and separators.
 *
 * Usage:
 *     auto tb = make<Toolbar>();
 *     tb->addButton(IconName::Cut,  "Cut",   [&]{ doCut(); },  "Cut (Cmd+X)");
 *     tb->addButton(IconName::Copy, "Copy",  [&]{ doCopy(); }, "Copy (Cmd+C)");
 *     tb->addSeparator();
 *     tb->addToggle(IconName::Bold, "B", isBold, [&](bool v){ setBold(v); }, "Bold");
 */
class GUT_API Toolbar : public Element {
    GUT_OBJECT(Toolbar, Element)

public:
    Toolbar();
    ~Toolbar() override = default;

    // -------------------------------------------------------------------------
    // Items
    // -------------------------------------------------------------------------

    struct Item {
        ToolbarItemType type{ToolbarItemType::Button};
        IconName icon{IconName::None};
        String label;
        String tooltip;
        std::function<void()> onClick;              // Button
        std::function<void(bool)> onToggled;        // Toggle
        bool toggled{false};
        bool enabled{true};
    };

    /// Add a button with icon and/or label, optional tooltip.
    void addButton(IconName icon, String label, std::function<void()> action,
                   String tooltip = "");

    /// Add an icon-only button.
    void addButton(IconName icon, std::function<void()> action, String tooltip = "");

    /// Add a label-only button.
    void addButton(String label, std::function<void()> action, String tooltip = "");

    /// Add a toggle button (icon + optional label).
    void addToggle(IconName icon, String label, bool initialState,
                   std::function<void(bool)> onToggled, String tooltip = "");

    /// Add a vertical separator.
    void addSeparator();

    /// Access items for runtime modification.
    usize itemCount() const { return m_items.size(); }
    Item& itemAt(usize index) { return m_items[index]; }
    const Item& itemAt(usize index) const { return m_items[index]; }

    /// Set the toggled state of a toggle item by index.
    void setToggled(usize index, bool value);

    /// Enable / disable an item by index.
    void setItemEnabled(usize index, bool enabled);

    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------

    GUT_PROPERTY(f32, itemHeight, 32.0f)
    GUT_PROPERTY(f32, itemPadding, 6.0f)     // horizontal padding inside each item
    GUT_PROPERTY(f32, iconSize, 16.0f)
    GUT_PROPERTY(f32, fontSize, 11.0f)
    GUT_PROPERTY(f32, separatorWidth, 1.0f)
    GUT_PROPERTY(f32, spacing, 2.0f)         // gap between items

    GUT_PROPERTY(Color, toolbarBackground, Color::fromHex(0x1E1E2A))
    GUT_PROPERTY(Color, itemHoverBackground, Color::fromRgba8(55, 55, 70, 255))
    GUT_PROPERTY(Color, itemPressedBackground, Color::fromRgba8(40, 85, 180, 255))
    GUT_PROPERTY(Color, itemToggledBackground, Color::fromRgba8(50, 100, 200, 200))
    GUT_PROPERTY(Color, itemForeground, Color::fromRgba8(200, 200, 220, 255))
    GUT_PROPERTY(Color, itemDisabledForeground, Color::fromRgba8(90, 90, 110, 255))
    GUT_PROPERTY(Color, separatorColor, Color::fromRgba8(60, 60, 78, 180))
    GUT_PROPERTY(Color, borderColor, Color::fromRgba8(50, 50, 65, 200))

protected:
    Size2f measureOverride(Size2f availableSize) override;
    void onRender(RenderContext& ctx) override;
    bool onMouseEvent(const MouseEvent& event) override;
    void onMouseEnter() override;
    void onMouseLeave() override;

private:
    std::vector<Item> m_items;
    isize m_hoveredItem{-1};
    isize m_pressedItem{-1};

    struct ItemLayout {
        Rectf rect;
        usize itemIndex;
    };
    std::vector<ItemLayout> m_layout; // computed per frame

    void computeLayout();
    f32 itemWidth(const Item& item) const;
    isize itemIndexAtX(f32 x) const;
};

} // namespace gut


// --- gut/elements/Image.h ---



namespace gut {

// Forward declaration
class Texture;

/**
 * @brief Element that displays an image.
 */
class GUT_API Image : public Element {
    GUT_OBJECT(Image, Element)
    
public:
    Image() = default;
    explicit Image(Ref<Texture> texture);
    ~Image() override = default;
    
    // -------------------------------------------------------------------------
    // Content
    // -------------------------------------------------------------------------
    
    void setTexture(Ref<Texture> texture);
    Texture* texture() const { return m_texture.get(); }
    
    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------
    
    enum class Stretch {
        None,           // Display at natural size
        Fill,           // Fill the entire area (may distort)
        Uniform,        // Scale to fit, preserving aspect ratio
        UniformToFill,  // Scale to fill, preserving aspect ratio (may clip)
        NineSlice       // 9-slice scaling — corners fixed, edges stretch
    };
    
    GUT_PROPERTY(Stretch, stretch, Stretch::Uniform)
    
    /**
     * @brief Border insets for NineSlice stretch mode.
     * Defines the pixel distances from each edge that delimit the 9 slices.
     */
    GUT_PROPERTY(Thickness, sliceBorders, Thickness{})
    GUT_PROPERTY(HorizontalAlignment, horizontalImageAlignment, HorizontalAlignment::Center)
    GUT_PROPERTY(VerticalAlignment, verticalImageAlignment, VerticalAlignment::Center)
    GUT_PROPERTY(Color, tint, Color::white())  // Color multiplier
    
    // -------------------------------------------------------------------------
    // Source rectangle (for sprite sheets)
    // -------------------------------------------------------------------------
    
    /**
     * @brief Set the source rectangle within the texture (for sprite sheets).
     * If not set, the entire texture is used.
     */
    void setSourceRect(Rectf rect);
    const Rectf& sourceRect() const { return m_sourceRect; }
    bool hasSourceRect() const { return m_hasSourceRect; }
    void clearSourceRect();

protected:
    Size2f measureOverride(Size2f availableSize) override;
    void onRender(RenderContext& ctx) override;

private:
    Ref<Texture> m_texture;
    Rectf m_sourceRect;
    bool m_hasSourceRect{false};
};

} // namespace gut


// --- gut/elements/ScrollViewer.h ---



namespace gut {

/**
 * @brief Scrollbar visibility mode.
 */
enum class ScrollBarVisibility {
    Auto,       // Show when needed
    Visible,    // Always visible
    Hidden,     // Never visible (can still scroll programmatically)
    Disabled    // No scrolling
};

/**
 * @brief Panel that provides scrolling for content larger than the viewport.
 */
class GUT_API ScrollViewer : public Panel {
    GUT_OBJECT(ScrollViewer, Panel)
    
public:
    ScrollViewer() = default;
    ~ScrollViewer() override = default;
    
    // -------------------------------------------------------------------------
    // Scrollbar settings
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(ScrollBarVisibility, horizontalScrollBarVisibility, ScrollBarVisibility::Auto)
    GUT_PROPERTY(ScrollBarVisibility, verticalScrollBarVisibility, ScrollBarVisibility::Auto)
    
    // -------------------------------------------------------------------------
    // Scroll position
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(f32, horizontalOffset, 0.0f)
    GUT_PROPERTY(f32, verticalOffset, 0.0f)
    
    /**
     * @brief Get the total scrollable width.
     */
    f32 scrollableWidth() const { return m_scrollableWidth; }
    
    /**
     * @brief Get the total scrollable height.
     */
    f32 scrollableHeight() const { return m_scrollableHeight; }
    
    /**
     * @brief Get the viewport width.
     */
    f32 viewportWidth() const { return m_viewportWidth; }
    
    /**
     * @brief Get the viewport height.
     */
    f32 viewportHeight() const { return m_viewportHeight; }
    
    // -------------------------------------------------------------------------
    // Scroll methods
    // -------------------------------------------------------------------------
    
    /**
     * @brief Scroll to a specific position.
     */
    void scrollTo(f32 horizontalOffset, f32 verticalOffset);
    
    /**
     * @brief Scroll by a delta amount.
     */
    void scrollBy(f32 deltaX, f32 deltaY);
    
    /**
     * @brief Scroll to make an element visible.
     */
    void scrollIntoView(Element* element);
    
    /**
     * @brief Scroll to the top.
     */
    void scrollToTop();
    
    /**
     * @brief Scroll to the bottom.
     */
    void scrollToBottom();
    
    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------
    
    GUT_PROPERTY(f32, scrollBarWidth, 12.0f)
    GUT_PROPERTY(Color, scrollBarBackground, Color::fromHex(0xF0F0F0))
    GUT_PROPERTY(Color, scrollBarThumb, Color::fromHex(0xC0C0C0))
    GUT_PROPERTY(Color, scrollBarThumbHover, Color::fromHex(0xA0A0A0))
    GUT_PROPERTY(Color, scrollBarThumbDrag, Color::fromHex(0x808080))
    GUT_PROPERTY(f32, scrollArrowSize, 14.0f)

protected:
    Size2f measureOverride(Size2f availableSize) override;
    Size2f arrangeOverride(Size2f finalSize) override;
    void onRender(RenderContext& ctx) override;
    bool onMouseEvent(const MouseEvent& event) override;
    void onMouseLeave() override;

private:
    void updateScrollBars();
    bool needsHorizontalScrollBar() const;
    bool needsVerticalScrollBar() const;
    
    // Thumb geometry helpers
    Rectf verticalThumbRect() const;
    Rectf verticalTrackRect() const;
    Rectf verticalUpArrowRect() const;
    Rectf verticalDownArrowRect() const;
    
    f32 m_scrollableWidth{0.0f};
    f32 m_scrollableHeight{0.0f};
    f32 m_viewportWidth{0.0f};
    f32 m_viewportHeight{0.0f};
    
    bool m_horizontalThumbHovered{false};
    bool m_verticalThumbHovered{false};
    bool m_draggingHorizontal{false};
    bool m_draggingVertical{false};
    f32 m_dragStartOffset{0.0f};
    f32 m_dragStartMouse{0.0f};
    
    bool m_upArrowHovered{false};
    bool m_downArrowHovered{false};
    bool m_upArrowPressed{false};
    bool m_downArrowPressed{false};
    bool m_pendingScrollToBottom{false};
};

} // namespace gut


// --- gut/elements/TextBox.h ---


#include <string>
#include <algorithm>

namespace gut {

/**
 * @brief Single-line text input control.
 *
 * Supports typing, caret movement, text selection, clipboard (Cmd+C/V/X),
 * Home/End, Ctrl+Arrows for word navigation, and a blinking caret.
 */
class GUT_API TextBox : public Element {
    GUT_OBJECT(TextBox, Element)

public:
    TextBox();
    explicit TextBox(String initialText);
    ~TextBox() override = default;

    // -------------------------------------------------------------------------
    // Content
    // -------------------------------------------------------------------------

    GUT_PROPERTY(String, text, "")
    GUT_PROPERTY(String, placeholder, "")

    // -------------------------------------------------------------------------
    // Appearance
    // -------------------------------------------------------------------------

    GUT_PROPERTY(Color, background, Color::white())
    GUT_PROPERTY(Color, foreground, Color::black())
    GUT_PROPERTY(Color, placeholderColor, Color::fromRgba8(160, 160, 160))
    GUT_PROPERTY(Color, borderColor, Color::fromHex(0xA0A0A0))
    GUT_PROPERTY(Color, focusBorderColor, Color::fromHex(0x0078D4))
    GUT_PROPERTY(Color, caretColor, Color::black())
    GUT_PROPERTY(Color, selectionColor, Color::fromRgba8(0, 120, 212, 80))
    GUT_PROPERTY(f32, borderWidth, 1.0f)
    GUT_PROPERTY(f32, cornerRadius, 4.0f)
    GUT_PROPERTY(f32, fontSize, 14.0f)
    GUT_PROPERTY(bool, readOnly, false)
    GUT_PROPERTY(bool, isPassword, false)
    GUT_PROPERTY(char, passwordChar, '\xE2')  // bullet char placeholder, we use Unicode below

    // -------------------------------------------------------------------------
    // Selection
    // -------------------------------------------------------------------------

    /// Caret position (0 = before first char, text.length() = after last char)
    i32 caretPosition() const { return m_caretPos; }
    void setCaretPosition(i32 pos);

    /// Selection anchor (where selection started); -1 = no selection
    i32 selectionStart() const;
    i32 selectionEnd() const;
    bool hasSelection() const { return m_selAnchor >= 0 && m_selAnchor != m_caretPos; }
    String selectedText() const;

    /// Select a range
    void select(i32 start, i32 end);
    void selectAll();
    void clearSelection();

    // -------------------------------------------------------------------------
    // Signals
    // -------------------------------------------------------------------------

    Signal<const String&> textChanged;  // emitted on every edit
    Signal<> submitted;                 // emitted on Return/Enter

    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------

    void setOnTextChanged(std::function<void(const String&)> cb) { m_onTextChanged = std::move(cb); }
    void setOnSubmit(std::function<void()> cb) { m_onSubmit = std::move(cb); }

protected:
    Size2f measureOverride(Size2f availableSize) override;
    void onRender(RenderContext& ctx) override;
    bool onMouseEvent(const MouseEvent& event) override;
    bool onKeyEvent(const KeyEvent& event) override;
    void onFocusGained() override;
    void onFocusLost() override;
    void onMouseEnter() override;
    void onMouseLeave() override;

private:
    /// Returns the display string (bullets for password mode, real text otherwise)
    String displayText() const;

    /// Insert text at caret, replacing selection if any
    void insertText(const String& str);
    /// Delete selection or N chars (negative = before caret, positive = after)
    void deleteText(i32 count);
    /// Move caret, optionally extending selection
    void moveCaret(i32 newPos, bool selecting);
    /// Find the character index closest to a local x coordinate
    i32 hitTestCaret(f32 localX) const;
    /// Measure the width of text[0..pos)
    f32 measureSubstring(i32 pos) const;
    /// Ensure caret is scrolled into view
    void scrollToCaret();

    // Word boundary helpers
    i32 wordBoundaryLeft(i32 pos) const;
    i32 wordBoundaryRight(i32 pos) const;

    // Caret
    i32 m_caretPos{0};
    i32 m_selAnchor{-1};  // -1 = no selection
    f32 m_scrollOffset{0.0f};  // horizontal scroll for long text
    f32 m_caretBlinkTimer{0.0f};
    bool m_caretVisible{true};

    // Callbacks
    std::function<void(const String&)> m_onTextChanged;
    std::function<void()> m_onSubmit;
};

} // namespace gut


// --- gut/Context.h ---


#include <memory>

namespace gut {

/**
 * @brief Main context for a gut UI instance.
 * 
 * The Context owns:
 * - The root element tree
 * - The render backend and context
 * - Input management
 * - Focus management
 * - Stylesheets and theming
 * 
 * Create one Context per UI view/window in your application.
 */
class GUT_API Context {
public:
    /**
     * @brief Create a context with a render backend.
     */
    explicit Context(std::unique_ptr<RenderBackend> backend);
    
    ~Context();
    
    GUT_NONCOPYABLE(Context)
    
    // -------------------------------------------------------------------------
    // Root element
    // -------------------------------------------------------------------------
    
    /**
     * @brief Set the root element of the UI tree.
     */
    void setRoot(Ref<Element> root);
    
    /**
     * @brief Get the root element.
     */
    Element* root() const { return m_root.get(); }
    
    // -------------------------------------------------------------------------
    // Styling
    // -------------------------------------------------------------------------
    
    /**
     * @brief Add a stylesheet.
     */
    void addStyleSheet(Ref<StyleSheet> stylesheet);
    
    /**
     * @brief Set the theme.
     */
    void setTheme(Ref<Theme> theme);
    
    /**
     * @brief Get the current theme.
     */
    Theme* theme() const { return m_theme.get(); }
    
    // -------------------------------------------------------------------------
    // Update and render
    // -------------------------------------------------------------------------
    
    /**
     * @brief Update the UI (animations, timers, etc.)
     * @param deltaTime Time since last update in seconds.
     */
    void update(f32 deltaTime);
    
    /**
     * @brief Perform layout if needed.
     */
    void layout();
    
    /**
     * @brief Render the UI.
     * @param width Viewport width.
     * @param height Viewport height.
     * @param devicePixelRatio Device pixel ratio (default 1.0).
     */
    void render(f32 width, f32 height, f32 devicePixelRatio = 1.0f);
    
    // -------------------------------------------------------------------------
    // Input (convenience methods that delegate to InputManager)
    // -------------------------------------------------------------------------
    
    void processMouseMove(f32 x, f32 y);
    void processMouseButton(MouseButton button, bool pressed);
    void processMouseWheel(f32 deltaX, f32 deltaY);
    void processKey(Key key, bool pressed, ModifierKeys modifiers = ModifierKeys::None);
    void processTextInput(char32_t character);
    
    // -------------------------------------------------------------------------
    // Managers
    // -------------------------------------------------------------------------
    
    InputManager& inputManager() { return *m_inputManager; }
    FocusManager& focusManager() { return *m_focusManager; }
    RenderBackend& renderBackend() { return *m_backend; }
    
    // -------------------------------------------------------------------------
    // Element lookup
    // -------------------------------------------------------------------------
    
    /**
     * @brief Find an element by ID.
     */
    Element* findById(StringView id);
    
    /**
     * @brief Find elements by class name.
     */
    std::vector<Element*> findByClass(StringView className);
    
    /**
     * @brief Hit test at a point.
     */
    Element* hitTest(Point2f point);
    
    // -------------------------------------------------------------------------
    // Size
    // -------------------------------------------------------------------------
    
    /**
     * @brief Get the current viewport size.
     */
    Size2f size() const { return m_size; }
    
    /**
     * @brief Set the viewport size (also invalidates layout).
     */
    void setSize(f32 width, f32 height);

    // -------------------------------------------------------------------------
    // Overlays (popups, tooltips rendered on top of everything)
    // -------------------------------------------------------------------------

    /**
     * @brief Register an overlay render callback.
     * Overlays are rendered after the entire element tree, so they appear
     * on top of all siblings and ignore parent clipping.
     * @param owner The element that owns this overlay (used as a key).
     * @param renderFn Callback invoked with a clean RenderContext (identity transform).
     */
    void addOverlay(Element* owner, std::function<void(RenderContext&)> renderFn);

    /**
     * @brief Remove the overlay owned by the given element.
     */
    void removeOverlay(Element* owner);

    // -------------------------------------------------------------------------
    // Time
    // -------------------------------------------------------------------------
    
    /**
     * @brief Get the total elapsed time since context creation.
     */
    f64 totalTime() const { return m_totalTime; }

    // -------------------------------------------------------------------------
    // Cursor
    // -------------------------------------------------------------------------

    /**
     * @brief Set a callback that is invoked whenever the cursor style should change.
     * The platform layer should map CursorType to native cursors (e.g., NSCursor).
     */
    void setOnCursorChanged(std::function<void(CursorType)> callback) {
        m_onCursorChanged = std::move(callback);
    }

    /**
     * @brief Get the current cursor type.
     */
    CursorType currentCursor() const { return m_currentCursor; }

    /**
     * @brief Notify that the cursor should change (called by InputManager).
     */
    void notifyCursorChanged(CursorType cursor) {
        if (cursor != m_currentCursor) {
            m_currentCursor = cursor;
            if (m_onCursorChanged) {
                m_onCursorChanged(cursor);
            }
        }
    }
    
    // -------------------------------------------------------------------------
    // Font loading
    // -------------------------------------------------------------------------
    
    /**
     * @brief Load a font from memory (raw .ttf/.otf data).
     * @param data Pointer to font file data.
     * @param size Size of font data in bytes.
     * @return Loaded font, or nullptr on failure.
     *
     * The font's family, weight and style are auto-detected from the
     * TrueType/OpenType tables and the font is automatically registered
     * in the font registry.  Call Font::setFamily / setWeight / setStyle
     * before loading if you need to override the detected values.
     */
    Ref<Font> loadFont(const u8* data, size_t size);
    
    /**
     * @brief Load a font with explicit family, weight and style metadata.
     */
    Ref<Font> loadFont(const u8* data, size_t size,
                       const std::string& family,
                       FontWeight weight = FontWeight::Normal,
                       FontStyle  style  = FontStyle::Normal);
    
    /**
     * @brief Register a previously-loaded font in the font registry.
     *
     * The font's family(), weight() and style() are used as the registry key.
     * If a font with the same key already exists it is replaced.
     */
    void registerFont(Ref<Font> font);
    
    /**
     * @brief Find a font by family, weight and style.
     *
     * Performs a best-match lookup:
     *  1. Exact match on (family, weight, style).
     *  2. Same family, nearest weight, same style.
     *  3. Same family, any weight/style.
     *  4. Falls back to defaultFont().
     */
    Font* findFont(const std::string& family,
                   FontWeight weight = FontWeight::Normal,
                   FontStyle  style  = FontStyle::Normal) const;
    
    /**
     * @brief Set the default font for text rendering.
     */
    void setDefaultFont(Ref<Font> font);
    
    /**
     * @brief Get the default font.
     */
    Font* defaultFont() const { return m_defaultFont.get(); }

private:
    void applyStyles();
    void applyStyleToElement(Element* element);
    void renderElement(Element* element);
    Element* findElementById(Element* element, StringView id);
    void findElementsByClass(Element* element, StringView className, 
                              std::vector<Element*>& results);
    Element* hitTestElement(Element* element, Point2f point);
    void propagateContext(Element* element);
    
    std::unique_ptr<RenderBackend> m_backend;
    std::unique_ptr<RenderContext> m_renderContext;
    std::unique_ptr<InputManager> m_inputManager;
    std::unique_ptr<FocusManager> m_focusManager;
    
    Ref<Element> m_root;
    Ref<Theme> m_theme;
    Ref<Font> m_defaultFont;
    std::vector<Ref<StyleSheet>> m_styleSheets;
    
    // Font registry — maps (family, weight, style) → Font
    struct FontKey {
        std::string family;
        FontWeight weight;
        FontStyle style;
        bool operator==(const FontKey& o) const {
            return family == o.family && weight == o.weight && style == o.style;
        }
    };
    struct FontKeyHash {
        size_t operator()(const FontKey& k) const {
            size_t h = std::hash<std::string>{}(k.family);
            h ^= std::hash<int>{}(static_cast<int>(k.weight)) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<int>{}(static_cast<int>(k.style))  + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };
    std::unordered_map<FontKey, Ref<Font>, FontKeyHash> m_fontRegistry;
    
    Size2f m_size{};
    f64 m_totalTime{0.0};
    bool m_layoutDirty{true};

    // Cursor
    CursorType m_currentCursor{CursorType::Arrow};
    std::function<void(CursorType)> m_onCursorChanged;

    // Overlay render callbacks (popups, tooltips, etc.)
    std::vector<std::pair<Element*, std::function<void(RenderContext&)>>> m_overlays;
};

} // namespace gut


// --- gut/gut.h ---


/**
 * @file gut.h
 * @brief Main header for the Gut Game UI Toolkit.
 * 
 * Include this header to access all gut functionality:
 * 
 * @code
 * #include <gut/gut.h>
 * 
 * int main() {
 *     auto backend = std::make_unique<MyOpenGLBackend>();
 *     gut::Context ctx(std::move(backend));
 *     
 *     auto root = gut::make<gut::StackPanel>(gut::Orientation::Vertical, {
 *         gut::make<gut::Text>("Hello, Gut!"),
 *         gut::make<gut::Button>("Click Me", [] {
 *             std::cout << "Clicked!\n";
 *         })
 *     });
 *     
 *     ctx.setRoot(root);
 *     
 *     while (running) {
 *         ctx.processMouseMove(mouseX, mouseY);
 *         ctx.update(deltaTime);
 *         ctx.render(width, height);
 *     }
 * }
 * @endcode
 */

// Export and types

// Core

// Context

// Elements

// Styling

// Input

// Rendering

// Resources

// Animation

/**
 * @namespace gut
 * @brief The Gut Game UI Toolkit namespace.
 * 
 * All gut types and functions are contained within this namespace.
 */
namespace gut {

/**
 * @brief Get the gut library major version number.
 */
GUT_API i32 versionMajor();

/**
 * @brief Get the gut library minor version number.
 */
GUT_API i32 versionMinor();

/**
 * @brief Get the gut library patch version number.
 */
GUT_API i32 versionPatch();

/**
 * @brief Get the gut library version string (e.g., "1.0.0").
 */
GUT_API const char* versionString();

/**
 * @brief Initialize the gut library.
 * @return true if initialization succeeded
 */
GUT_API bool initialize();

/**
 * @brief Shutdown the gut library and release resources.
 */
GUT_API void shutdown();

} // namespace gut



#endif // GUT_SINGLE_HPP

// ============================================================================
// IMPLEMENTATION
// ============================================================================
#ifdef GUT_IMPLEMENTATION

// --- Embedded stb_truetype.h (v1.26, public domain by Sean Barrett) ---
// Inlined here so gut_single.h is fully self-contained.
// If the host app already included stb_truetype.h, we skip our copy entirely.
#ifndef __STB_INCLUDE_STB_TRUETYPE_H__
#define GUT_OWNS_STB_TRUETYPE
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC

// Begin stb_truetype.h --------------------------------------------------------
// stb_truetype.h - v1.26 - public domain
// authored from 2009-2021 by Sean Barrett / RAD Game Tools
//
// =======================================================================
//
//    NO SECURITY GUARANTEE -- DO NOT USE THIS ON UNTRUSTED FONT FILES
//
// This library does no range checking of the offsets found in the file,
// meaning an attacker can use it to read arbitrary memory.
//
// =======================================================================
//
//   This library processes TrueType files:
//        parse files
//        extract glyph metrics
//        extract glyph shapes
//        render glyphs to one-channel bitmaps with antialiasing (box filter)
//        render glyphs to one-channel SDF bitmaps (signed-distance field/function)
//
//   Todo:
//        non-MS cmaps
//        crashproof on bad data
//        hinting? (no longer patented)
//        cleartype-style AA?
//        optimize: use simple memory allocator for intermediates
//        optimize: build edge-list directly from curves
//        optimize: rasterize directly from curves?
//
// ADDITIONAL CONTRIBUTORS
//
//   Mikko Mononen: compound shape support, more cmap formats
//   Tor Andersson: kerning, subpixel rendering
//   Dougall Johnson: OpenType / Type 2 font handling
//   Daniel Ribeiro Maciel: basic GPOS-based kerning
//
//   Misc other:
//       Ryan Gordon
//       Simon Glass
//       github:IntellectualKitty
//       Imanol Celaya
//       Daniel Ribeiro Maciel
//
//   Bug/warning reports/fixes:
//       "Zer" on mollyrocket       Fabian "ryg" Giesen   github:NiLuJe
//       Cass Everitt               Martins Mozeiko       github:aloucks
//       stoiko (Haemimont Games)   Cap Petschulat        github:oyvindjam
//       Brian Hook                 Omar Cornut           github:vassvik
//       Walter van Niftrik         Ryan Griege
//       David Gow                  Peter LaValle
//       David Given                Sergey Popov
//       Ivan-Assen Ivanov          Giumo X. Clanjor
//       Anthony Pesch              Higor Euripedes
//       Johan Duparc               Thomas Fields
//       Hou Qiming                 Derek Vinyard
//       Rob Loach                  Cort Stratton
//       Kenney Phillis Jr.         Brian Costabile
//       Ken Voskuil (kaesve)       Yakov Galka
//
// VERSION HISTORY
//
//   1.26 (2021-08-28) fix broken rasterizer
//   1.25 (2021-07-11) many fixes
//   1.24 (2020-02-05) fix warning
//   1.23 (2020-02-02) query SVG data for glyphs; query whole kerning table (but only kern not GPOS)
//   1.22 (2019-08-11) minimize missing-glyph duplication; fix kerning if both 'GPOS' and 'kern' are defined
//   1.21 (2019-02-25) fix warning
//   1.20 (2019-02-07) PackFontRange skips missing codepoints; GetScaleFontVMetrics()
//   1.19 (2018-02-11) GPOS kerning, STBTT_fmod
//   1.18 (2018-01-29) add missing function
//   1.17 (2017-07-23) make more arguments const; doc fix
//   1.16 (2017-07-12) SDF support
//   1.15 (2017-03-03) make more arguments const
//   1.14 (2017-01-16) num-fonts-in-TTC function
//   1.13 (2017-01-02) support OpenType fonts, certain Apple fonts
//   1.12 (2016-10-25) suppress warnings about casting away const with -Wcast-qual
//   1.11 (2016-04-02) fix unused-variable warning
//   1.10 (2016-04-02) user-defined fabs(); rare memory leak; remove duplicate typedef
//   1.09 (2016-01-16) warning fix; avoid crash on outofmem; use allocation userdata properly
//   1.08 (2015-09-13) document stbtt_Rasterize(); fixes for vertical & horizontal edges
//   1.07 (2015-08-01) allow PackFontRanges to accept arrays of sparse codepoints;
//                     variant PackFontRanges to pack and render in separate phases;
//                     fix stbtt_GetFontOFfsetForIndex (never worked for non-0 input?);
//                     fixed an assert() bug in the new rasterizer
//                     replace assert() with STBTT_assert() in new rasterizer
//
//   Full history can be found at the end of this file.
//
// LICENSE
//
//   See end of file for license information.
//
// USAGE
//
//   Include this file in whatever places need to refer to it. In ONE C/C++
//   file, write:
//      #define STB_TRUETYPE_IMPLEMENTATION
//   before the #include of this file. This expands out the actual
//   implementation into that C/C++ file.
//
//   To make the implementation private to the file that generates the implementation,
//      #define STBTT_STATIC
//
//   Simple 3D API (don't ship this, but it's fine for tools and quick start)
//           stbtt_BakeFontBitmap()               -- bake a font to a bitmap for use as texture
//           stbtt_GetBakedQuad()                 -- compute quad to draw for a given char
//
//   Improved 3D API (more shippable):
//           #include "stb_rect_pack.h"           -- optional, but you really want it
//           stbtt_PackBegin()
//           stbtt_PackSetOversampling()          -- for improved quality on small fonts
//           stbtt_PackFontRanges()               -- pack and renders
//           stbtt_PackEnd()
//           stbtt_GetPackedQuad()
//
//   "Load" a font file from a memory buffer (you have to keep the buffer loaded)
//           stbtt_InitFont()
//           stbtt_GetFontOffsetForIndex()        -- indexing for TTC font collections
//           stbtt_GetNumberOfFonts()             -- number of fonts for TTC font collections
//
//   Render a unicode codepoint to a bitmap
//           stbtt_GetCodepointBitmap()           -- allocates and returns a bitmap
//           stbtt_MakeCodepointBitmap()          -- renders into bitmap you provide
//           stbtt_GetCodepointBitmapBox()        -- how big the bitmap must be
//
//   Character advance/positioning
//           stbtt_GetCodepointHMetrics()
//           stbtt_GetFontVMetrics()
//           stbtt_GetFontVMetricsOS2()
//           stbtt_GetCodepointKernAdvance()
//
//   Starting with version 1.06, the rasterizer was replaced with a new,
//   faster and generally-more-precise rasterizer. The new rasterizer more
//   accurately measures pixel coverage for anti-aliasing, except in the case
//   where multiple shapes overlap, in which case it overestimates the AA pixel
//   coverage. Thus, anti-aliasing of intersecting shapes may look wrong. If
//   this turns out to be a problem, you can re-enable the old rasterizer with
//        #define STBTT_RASTERIZER_VERSION 1
//   which will incur about a 15% speed hit.
//
// ADDITIONAL DOCUMENTATION
//
//   Immediately after this block comment are a series of sample programs.
//
//   After the sample programs is the "header file" section. This section
//   includes documentation for each API function.
//
//   Some important concepts to understand to use this library:
//
//      Codepoint
//         Characters are defined by unicode codepoints, e.g. 65 is
//         uppercase A, 231 is lowercase c with a cedilla, 0x7e30 is
//         the hiragana for "ma".
//
//      Glyph
//         A visual character shape (every codepoint is rendered as
//         some glyph)
//
//      Glyph index
//         A font-specific integer ID representing a glyph
//
//      Baseline
//         Glyph shapes are defined relative to a baseline, which is the
//         bottom of uppercase characters. Characters extend both above
//         and below the baseline.
//
//      Current Point
//         As you draw text to the screen, you keep track of a "current point"
//         which is the origin of each character. The current point's vertical
//         position is the baseline. Even "baked fonts" use this model.
//
//      Vertical Font Metrics
//         The vertical qualities of the font, used to vertically position
//         and space the characters. See docs for stbtt_GetFontVMetrics.
//
//      Font Size in Pixels or Points
//         The preferred interface for specifying font sizes in stb_truetype
//         is to specify how tall the font's vertical extent should be in pixels.
//         If that sounds good enough, skip the next paragraph.
//
//         Most font APIs instead use "points", which are a common typographic
//         measurement for describing font size, defined as 72 points per inch.
//         stb_truetype provides a point API for compatibility. However, true
//         "per inch" conventions don't make much sense on computer displays
//         since different monitors have different number of pixels per
//         inch. For example, Windows traditionally uses a convention that
//         there are 96 pixels per inch, thus making 'inch' measurements have
//         nothing to do with inches, and thus effectively defining a point to
//         be 1.333 pixels. Additionally, the TrueType font data provides
//         an explicit scale factor to scale a given font's glyphs to points,
//         but the author has observed that this scale factor is often wrong
//         for non-commercial fonts, thus making fonts scaled in points
//         according to the TrueType spec incoherently sized in practice.
//
// DETAILED USAGE:
//
//  Scale:
//    Select how high you want the font to be, in points or pixels.
//    Call ScaleForPixelHeight or ScaleForMappingEmToPixels to compute
//    a scale factor SF that will be used by all other functions.
//
//  Baseline:
//    You need to select a y-coordinate that is the baseline of where
//    your text will appear. Call GetFontBoundingBox to get the baseline-relative
//    bounding box for all characters. SF*-y0 will be the distance in pixels
//    that the worst-case character could extend above the baseline, so if
//    you want the top edge of characters to appear at the top of the
//    screen where y=0, then you would set the baseline to SF*-y0.
//
//  Current point:
//    Set the current point where the first character will appear. The
//    first character could extend left of the current point; this is font
//    dependent. You can either choose a current point that is the leftmost
//    point and hope, or add some padding, or check the bounding box or
//    left-side-bearing of the first character to be displayed and set
//    the current point based on that.
//
//  Displaying a character:
//    Compute the bounding box of the character. It will contain signed values
//    relative to <current_point, baseline>. I.e. if it returns x0,y0,x1,y1,
//    then the character should be displayed in the rectangle from
//    <current_point+SF*x0, baseline+SF*y0> to <current_point+SF*x1,baseline+SF*y1).
//
//  Advancing for the next character:
//    Call GlyphHMetrics, and compute 'current_point += SF * advance'.
//
//
// ADVANCED USAGE
//
//   Quality:
//
//    - Use the functions with Subpixel at the end to allow your characters
//      to have subpixel positioning. Since the font is anti-aliased, not
//      hinted, this is very import for quality. (This is not possible with
//      baked fonts.)
//
//    - Kerning is now supported, and if you're supporting subpixel rendering
//      then kerning is worth using to give your text a polished look.
//
//   Performance:
//
//    - Convert Unicode codepoints to glyph indexes and operate on the glyphs;
//      if you don't do this, stb_truetype is forced to do the conversion on
//      every call.
//
//    - There are a lot of memory allocations. We should modify it to take
//      a temp buffer and allocate from the temp buffer (without freeing),
//      should help performance a lot.
//
// NOTES
//
//   The system uses the raw data found in the .ttf file without changing it
//   and without building auxiliary data structures. This is a bit inefficient
//   on little-endian systems (the data is big-endian), but assuming you're
//   caching the bitmaps or glyph shapes this shouldn't be a big deal.
//
//   It appears to be very hard to programmatically determine what font a
//   given file is in a general way. I provide an API for this, but I don't
//   recommend it.
//
//
// PERFORMANCE MEASUREMENTS FOR 1.06:
//
//                      32-bit     64-bit
//   Previous release:  8.83 s     7.68 s
//   Pool allocations:  7.72 s     6.34 s
//   Inline sort     :  6.54 s     5.65 s
//   New rasterizer  :  5.63 s     5.00 s

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
////
////  SAMPLE PROGRAMS
////
//
//  Incomplete text-in-3d-api example, which draws quads properly aligned to be lossless.
//  See "tests/truetype_demo_win32.c" for a complete version.
#if 0
#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

unsigned char ttf_buffer[1<<20];
unsigned char temp_bitmap[512*512];

stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
GLuint ftex;

void my_stbtt_initfont(void)
{
   fread(ttf_buffer, 1, 1<<20, fopen("c:/windows/fonts/times.ttf", "rb"));
   stbtt_BakeFontBitmap(ttf_buffer,0, 32.0, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!
   // can free ttf_buffer at this point
   glGenTextures(1, &ftex);
   glBindTexture(GL_TEXTURE_2D, ftex);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512,512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
   // can free temp_bitmap at this point
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void my_stbtt_print(float x, float y, char *text)
{
   // assume orthographic projection with units = screen pixels, origin at top left
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, ftex);
   glBegin(GL_QUADS);
   while (*text) {
      if (*text >= 32 && *text < 128) {
         stbtt_aligned_quad q;
         stbtt_GetBakedQuad(cdata, 512,512, *text-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9
         glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y0);
         glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y0);
         glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y1);
         glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y1);
      }
      ++text;
   }
   glEnd();
}
#endif
//
//
//////////////////////////////////////////////////////////////////////////////
//
// Complete program (this compiles): get a single bitmap, print as ASCII art
//
#if 0
#include <stdio.h>
#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

char ttf_buffer[1<<25];

int main(int argc, char **argv)
{
   stbtt_fontinfo font;
   unsigned char *bitmap;
   int w,h,i,j,c = (argc > 1 ? atoi(argv[1]) : 'a'), s = (argc > 2 ? atoi(argv[2]) : 20);

   fread(ttf_buffer, 1, 1<<25, fopen(argc > 3 ? argv[3] : "c:/windows/fonts/arialbd.ttf", "rb"));

   stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));
   bitmap = stbtt_GetCodepointBitmap(&font, 0,stbtt_ScaleForPixelHeight(&font, s), c, &w, &h, 0,0);

   for (j=0; j < h; ++j) {
      for (i=0; i < w; ++i)
         putchar(" .:ioVM@"[bitmap[j*w+i]>>5]);
      putchar('\n');
   }
   return 0;
}
#endif
//
// Output:
//
//     .ii.
//    @@@@@@.
//   V@Mio@@o
//   :i.  V@V
//     :oM@@M
//   :@@@MM@M
//   @@o  o@M
//  :@@.  M@M
//   @@@o@@@@
//   :M@@V:@@.
//
//////////////////////////////////////////////////////////////////////////////
//
// Complete program: print "Hello World!" banner, with bugs
//
#if 0
char buffer[24<<20];
unsigned char screen[20][79];

int main(int arg, char **argv)
{
   stbtt_fontinfo font;
   int i,j,ascent,baseline,ch=0;
   float scale, xpos=2; // leave a little padding in case the character extends left
   char *text = "Heljo World!"; // intentionally misspelled to show 'lj' brokenness

   fread(buffer, 1, 1000000, fopen("c:/windows/fonts/arialbd.ttf", "rb"));
   stbtt_InitFont(&font, buffer, 0);

   scale = stbtt_ScaleForPixelHeight(&font, 15);
   stbtt_GetFontVMetrics(&font, &ascent,0,0);
   baseline = (int) (ascent*scale);

   while (text[ch]) {
      int advance,lsb,x0,y0,x1,y1;
      float x_shift = xpos - (float) floor(xpos);
      stbtt_GetCodepointHMetrics(&font, text[ch], &advance, &lsb);
      stbtt_GetCodepointBitmapBoxSubpixel(&font, text[ch], scale,scale,x_shift,0, &x0,&y0,&x1,&y1);
      stbtt_MakeCodepointBitmapSubpixel(&font, &screen[baseline + y0][(int) xpos + x0], x1-x0,y1-y0, 79, scale,scale,x_shift,0, text[ch]);
      // note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
      // because this API is really for baking character bitmaps into textures. if you want to render
      // a sequence of characters, you really need to render each bitmap to a temp buffer, then
      // "alpha blend" that into the working buffer
      xpos += (advance * scale);
      if (text[ch+1])
         xpos += scale*stbtt_GetCodepointKernAdvance(&font, text[ch],text[ch+1]);
      ++ch;
   }

   for (j=0; j < 20; ++j) {
      for (i=0; i < 78; ++i)
         putchar(" .:ioVM@"[screen[j][i]>>5]);
      putchar('\n');
   }

   return 0;
}
#endif


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
////
////   INTEGRATION WITH YOUR CODEBASE
////
////   The following sections allow you to supply alternate definitions
////   of C library functions used by stb_truetype, e.g. if you don't
////   link with the C runtime library.

#ifdef STB_TRUETYPE_IMPLEMENTATION
   // #define your own (u)stbtt_int8/16/32 before including to override this
   #ifndef stbtt_uint8
   typedef unsigned char   stbtt_uint8;
   typedef signed   char   stbtt_int8;
   typedef unsigned short  stbtt_uint16;
   typedef signed   short  stbtt_int16;
   typedef unsigned int    stbtt_uint32;
   typedef signed   int    stbtt_int32;
   #endif

   typedef char stbtt__check_size32[sizeof(stbtt_int32)==4 ? 1 : -1];
   typedef char stbtt__check_size16[sizeof(stbtt_int16)==2 ? 1 : -1];

   // e.g. #define your own STBTT_ifloor/STBTT_iceil() to avoid math.h
   #ifndef STBTT_ifloor
   #include <math.h>
   #define STBTT_ifloor(x)   ((int) floor(x))
   #define STBTT_iceil(x)    ((int) ceil(x))
   #endif

   #ifndef STBTT_sqrt
   #include <math.h>
   #define STBTT_sqrt(x)      sqrt(x)
   #define STBTT_pow(x,y)     pow(x,y)
   #endif

   #ifndef STBTT_fmod
   #include <math.h>
   #define STBTT_fmod(x,y)    fmod(x,y)
   #endif

   #ifndef STBTT_cos
   #include <math.h>
   #define STBTT_cos(x)       cos(x)
   #define STBTT_acos(x)      acos(x)
   #endif

   #ifndef STBTT_fabs
   #include <math.h>
   #define STBTT_fabs(x)      fabs(x)
   #endif

   // #define your own functions "STBTT_malloc" / "STBTT_free" to avoid malloc.h
   #ifndef STBTT_malloc
   #include <stdlib.h>
   #define STBTT_malloc(x,u)  ((void)(u),malloc(x))
   #define STBTT_free(x,u)    ((void)(u),free(x))
   #endif

   #ifndef STBTT_assert
   #include <assert.h>
   #define STBTT_assert(x)    assert(x)
   #endif

   #ifndef STBTT_strlen
   #include <string.h>
   #define STBTT_strlen(x)    strlen(x)
   #endif

   #ifndef STBTT_memcpy
   #include <string.h>
   #define STBTT_memcpy       memcpy
   #define STBTT_memset       memset
   #endif
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////   INTERFACE
////
////

#ifndef __STB_INCLUDE_STB_TRUETYPE_H__
#define __STB_INCLUDE_STB_TRUETYPE_H__

#ifdef STBTT_STATIC
#define STBTT_DEF static
#else
#define STBTT_DEF extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

// private structure
typedef struct
{
   unsigned char *data;
   int cursor;
   int size;
} stbtt__buf;

//////////////////////////////////////////////////////////////////////////////
//
// TEXTURE BAKING API
//
// If you use this API, you only have to call two functions ever.
//

typedef struct
{
   unsigned short x0,y0,x1,y1; // coordinates of bbox in bitmap
   float xoff,yoff,xadvance;
} stbtt_bakedchar;

STBTT_DEF int stbtt_BakeFontBitmap(const unsigned char *data, int offset,  // font location (use offset=0 for plain .ttf)
                                float pixel_height,                     // height of font in pixels
                                unsigned char *pixels, int pw, int ph,  // bitmap to be filled in
                                int first_char, int num_chars,          // characters to bake
                                stbtt_bakedchar *chardata);             // you allocate this, it's num_chars long
// if return is positive, the first unused row of the bitmap
// if return is negative, returns the negative of the number of characters that fit
// if return is 0, no characters fit and no rows were used
// This uses a very crappy packing.

typedef struct
{
   float x0,y0,s0,t0; // top-left
   float x1,y1,s1,t1; // bottom-right
} stbtt_aligned_quad;

STBTT_DEF void stbtt_GetBakedQuad(const stbtt_bakedchar *chardata, int pw, int ph,  // same data as above
                               int char_index,             // character to display
                               float *xpos, float *ypos,   // pointers to current position in screen pixel space
                               stbtt_aligned_quad *q,      // output: quad to draw
                               int opengl_fillrule);       // true if opengl fill rule; false if DX9 or earlier
// Call GetBakedQuad with char_index = 'character - first_char', and it
// creates the quad you need to draw and advances the current position.
//
// The coordinate system used assumes y increases downwards.
//
// Characters will extend both above and below the current position;
// see discussion of "BASELINE" above.
//
// It's inefficient; you might want to c&p it and optimize it.

STBTT_DEF void stbtt_GetScaledFontVMetrics(const unsigned char *fontdata, int index, float size, float *ascent, float *descent, float *lineGap);
// Query the font vertical metrics without having to create a font first.


//////////////////////////////////////////////////////////////////////////////
//
// NEW TEXTURE BAKING API
//
// This provides options for packing multiple fonts into one atlas, not
// perfectly but better than nothing.

typedef struct
{
   unsigned short x0,y0,x1,y1; // coordinates of bbox in bitmap
   float xoff,yoff,xadvance;
   float xoff2,yoff2;
} stbtt_packedchar;

typedef struct stbtt_pack_context stbtt_pack_context;
typedef struct stbtt_fontinfo stbtt_fontinfo;
#ifndef STB_RECT_PACK_VERSION
typedef struct stbrp_rect stbrp_rect;
#endif

STBTT_DEF int  stbtt_PackBegin(stbtt_pack_context *spc, unsigned char *pixels, int width, int height, int stride_in_bytes, int padding, void *alloc_context);
// Initializes a packing context stored in the passed-in stbtt_pack_context.
// Future calls using this context will pack characters into the bitmap passed
// in here: a 1-channel bitmap that is width * height. stride_in_bytes is
// the distance from one row to the next (or 0 to mean they are packed tightly
// together). "padding" is the amount of padding to leave between each
// character (normally you want '1' for bitmaps you'll use as textures with
// bilinear filtering).
//
// Returns 0 on failure, 1 on success.

STBTT_DEF void stbtt_PackEnd  (stbtt_pack_context *spc);
// Cleans up the packing context and frees all memory.

#define STBTT_POINT_SIZE(x)   (-(x))

STBTT_DEF int  stbtt_PackFontRange(stbtt_pack_context *spc, const unsigned char *fontdata, int font_index, float font_size,
                                int first_unicode_char_in_range, int num_chars_in_range, stbtt_packedchar *chardata_for_range);
// Creates character bitmaps from the font_index'th font found in fontdata (use
// font_index=0 if you don't know what that is). It creates num_chars_in_range
// bitmaps for characters with unicode values starting at first_unicode_char_in_range
// and increasing. Data for how to render them is stored in chardata_for_range;
// pass these to stbtt_GetPackedQuad to get back renderable quads.
//
// font_size is the full height of the character from ascender to descender,
// as computed by stbtt_ScaleForPixelHeight. To use a point size as computed
// by stbtt_ScaleForMappingEmToPixels, wrap the point size in STBTT_POINT_SIZE()
// and pass that result as 'font_size':
//       ...,                  20 , ... // font max minus min y is 20 pixels tall
//       ..., STBTT_POINT_SIZE(20), ... // 'M' is 20 pixels tall

typedef struct
{
   float font_size;
   int first_unicode_codepoint_in_range;  // if non-zero, then the chars are continuous, and this is the first codepoint
   int *array_of_unicode_codepoints;       // if non-zero, then this is an array of unicode codepoints
   int num_chars;
   stbtt_packedchar *chardata_for_range; // output
   unsigned char h_oversample, v_oversample; // don't set these, they're used internally
} stbtt_pack_range;

STBTT_DEF int  stbtt_PackFontRanges(stbtt_pack_context *spc, const unsigned char *fontdata, int font_index, stbtt_pack_range *ranges, int num_ranges);
// Creates character bitmaps from multiple ranges of characters stored in
// ranges. This will usually create a better-packed bitmap than multiple
// calls to stbtt_PackFontRange. Note that you can call this multiple
// times within a single PackBegin/PackEnd.

STBTT_DEF void stbtt_PackSetOversampling(stbtt_pack_context *spc, unsigned int h_oversample, unsigned int v_oversample);
// Oversampling a font increases the quality by allowing higher-quality subpixel
// positioning, and is especially valuable at smaller text sizes.
//
// This function sets the amount of oversampling for all following calls to
// stbtt_PackFontRange(s) or stbtt_PackFontRangesGatherRects for a given
// pack context. The default (no oversampling) is achieved by h_oversample=1
// and v_oversample=1. The total number of pixels required is
// h_oversample*v_oversample larger than the default; for example, 2x2
// oversampling requires 4x the storage of 1x1. For best results, render
// oversampled textures with bilinear filtering. Look at the readme in
// stb/tests/oversample for information about oversampled fonts
//
// To use with PackFontRangesGather etc., you must set it before calls
// call to PackFontRangesGatherRects.

STBTT_DEF void stbtt_PackSetSkipMissingCodepoints(stbtt_pack_context *spc, int skip);
// If skip != 0, this tells stb_truetype to skip any codepoints for which
// there is no corresponding glyph. If skip=0, which is the default, then
// codepoints without a glyph recived the font's "missing character" glyph,
// typically an empty box by convention.

STBTT_DEF void stbtt_GetPackedQuad(const stbtt_packedchar *chardata, int pw, int ph,  // same data as above
                               int char_index,             // character to display
                               float *xpos, float *ypos,   // pointers to current position in screen pixel space
                               stbtt_aligned_quad *q,      // output: quad to draw
                               int align_to_integer);

STBTT_DEF int  stbtt_PackFontRangesGatherRects(stbtt_pack_context *spc, const stbtt_fontinfo *info, stbtt_pack_range *ranges, int num_ranges, stbrp_rect *rects);
STBTT_DEF void stbtt_PackFontRangesPackRects(stbtt_pack_context *spc, stbrp_rect *rects, int num_rects);
STBTT_DEF int  stbtt_PackFontRangesRenderIntoRects(stbtt_pack_context *spc, const stbtt_fontinfo *info, stbtt_pack_range *ranges, int num_ranges, stbrp_rect *rects);
// Calling these functions in sequence is roughly equivalent to calling
// stbtt_PackFontRanges(). If you more control over the packing of multiple
// fonts, or if you want to pack custom data into a font texture, take a look
// at the source to of stbtt_PackFontRanges() and create a custom version
// using these functions, e.g. call GatherRects multiple times,
// building up a single array of rects, then call PackRects once,
// then call RenderIntoRects repeatedly. This may result in a
// better packing than calling PackFontRanges multiple times
// (or it may not).

// this is an opaque structure that you shouldn't mess with which holds
// all the context needed from PackBegin to PackEnd.
struct stbtt_pack_context {
   void *user_allocator_context;
   void *pack_info;
   int   width;
   int   height;
   int   stride_in_bytes;
   int   padding;
   int   skip_missing;
   unsigned int   h_oversample, v_oversample;
   unsigned char *pixels;
   void  *nodes;
};

//////////////////////////////////////////////////////////////////////////////
//
// FONT LOADING
//
//

STBTT_DEF int stbtt_GetNumberOfFonts(const unsigned char *data);
// This function will determine the number of fonts in a font file.  TrueType
// collection (.ttc) files may contain multiple fonts, while TrueType font
// (.ttf) files only contain one font. The number of fonts can be used for
// indexing with the previous function where the index is between zero and one
// less than the total fonts. If an error occurs, -1 is returned.

STBTT_DEF int stbtt_GetFontOffsetForIndex(const unsigned char *data, int index);
// Each .ttf/.ttc file may have more than one font. Each font has a sequential
// index number starting from 0. Call this function to get the font offset for
// a given index; it returns -1 if the index is out of range. A regular .ttf
// file will only define one font and it always be at offset 0, so it will
// return '0' for index 0, and -1 for all other indices.

// The following structure is defined publicly so you can declare one on
// the stack or as a global or etc, but you should treat it as opaque.
struct stbtt_fontinfo
{
   void           * userdata;
   unsigned char  * data;              // pointer to .ttf file
   int              fontstart;         // offset of start of font

   int numGlyphs;                     // number of glyphs, needed for range checking

   int loca,head,glyf,hhea,hmtx,kern,gpos,svg; // table locations as offset from start of .ttf
   int index_map;                     // a cmap mapping for our chosen character encoding
   int indexToLocFormat;              // format needed to map from glyph index to glyph

   stbtt__buf cff;                    // cff font data
   stbtt__buf charstrings;            // the charstring index
   stbtt__buf gsubrs;                 // global charstring subroutines index
   stbtt__buf subrs;                  // private charstring subroutines index
   stbtt__buf fontdicts;              // array of font dicts
   stbtt__buf fdselect;               // map from glyph to fontdict
};

STBTT_DEF int stbtt_InitFont(stbtt_fontinfo *info, const unsigned char *data, int offset);
// Given an offset into the file that defines a font, this function builds
// the necessary cached info for the rest of the system. You must allocate
// the stbtt_fontinfo yourself, and stbtt_InitFont will fill it out. You don't
// need to do anything special to free it, because the contents are pure
// value data with no additional data structures. Returns 0 on failure.


//////////////////////////////////////////////////////////////////////////////
//
// CHARACTER TO GLYPH-INDEX CONVERSIOn

STBTT_DEF int stbtt_FindGlyphIndex(const stbtt_fontinfo *info, int unicode_codepoint);
// If you're going to perform multiple operations on the same character
// and you want a speed-up, call this function with the character you're
// going to process, then use glyph-based functions instead of the
// codepoint-based functions.
// Returns 0 if the character codepoint is not defined in the font.


//////////////////////////////////////////////////////////////////////////////
//
// CHARACTER PROPERTIES
//

STBTT_DEF float stbtt_ScaleForPixelHeight(const stbtt_fontinfo *info, float pixels);
// computes a scale factor to produce a font whose "height" is 'pixels' tall.
// Height is measured as the distance from the highest ascender to the lowest
// descender; in other words, it's equivalent to calling stbtt_GetFontVMetrics
// and computing:
//       scale = pixels / (ascent - descent)
// so if you prefer to measure height by the ascent only, use a similar calculation.

STBTT_DEF float stbtt_ScaleForMappingEmToPixels(const stbtt_fontinfo *info, float pixels);
// computes a scale factor to produce a font whose EM size is mapped to
// 'pixels' tall. This is probably what traditional APIs compute, but
// I'm not positive.

STBTT_DEF void stbtt_GetFontVMetrics(const stbtt_fontinfo *info, int *ascent, int *descent, int *lineGap);
// ascent is the coordinate above the baseline the font extends; descent
// is the coordinate below the baseline the font extends (i.e. it is typically negative)
// lineGap is the spacing between one row's descent and the next row's ascent...
// so you should advance the vertical position by "*ascent - *descent + *lineGap"
//   these are expressed in unscaled coordinates, so you must multiply by
//   the scale factor for a given size

STBTT_DEF int  stbtt_GetFontVMetricsOS2(const stbtt_fontinfo *info, int *typoAscent, int *typoDescent, int *typoLineGap);
// analogous to GetFontVMetrics, but returns the "typographic" values from the OS/2
// table (specific to MS/Windows TTF files).
//
// Returns 1 on success (table present), 0 on failure.

STBTT_DEF void stbtt_GetFontBoundingBox(const stbtt_fontinfo *info, int *x0, int *y0, int *x1, int *y1);
// the bounding box around all possible characters

STBTT_DEF void stbtt_GetCodepointHMetrics(const stbtt_fontinfo *info, int codepoint, int *advanceWidth, int *leftSideBearing);
// leftSideBearing is the offset from the current horizontal position to the left edge of the character
// advanceWidth is the offset from the current horizontal position to the next horizontal position
//   these are expressed in unscaled coordinates

STBTT_DEF int  stbtt_GetCodepointKernAdvance(const stbtt_fontinfo *info, int ch1, int ch2);
// an additional amount to add to the 'advance' value between ch1 and ch2

STBTT_DEF int stbtt_GetCodepointBox(const stbtt_fontinfo *info, int codepoint, int *x0, int *y0, int *x1, int *y1);
// Gets the bounding box of the visible part of the glyph, in unscaled coordinates

STBTT_DEF void stbtt_GetGlyphHMetrics(const stbtt_fontinfo *info, int glyph_index, int *advanceWidth, int *leftSideBearing);
STBTT_DEF int  stbtt_GetGlyphKernAdvance(const stbtt_fontinfo *info, int glyph1, int glyph2);
STBTT_DEF int  stbtt_GetGlyphBox(const stbtt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1);
// as above, but takes one or more glyph indices for greater efficiency

typedef struct stbtt_kerningentry
{
   int glyph1; // use stbtt_FindGlyphIndex
   int glyph2;
   int advance;
} stbtt_kerningentry;

STBTT_DEF int  stbtt_GetKerningTableLength(const stbtt_fontinfo *info);
STBTT_DEF int  stbtt_GetKerningTable(const stbtt_fontinfo *info, stbtt_kerningentry* table, int table_length);
// Retrieves a complete list of all of the kerning pairs provided by the font
// stbtt_GetKerningTable never writes more than table_length entries and returns how many entries it did write.
// The table will be sorted by (a.glyph1 == b.glyph1)?(a.glyph2 < b.glyph2):(a.glyph1 < b.glyph1)

//////////////////////////////////////////////////////////////////////////////
//
// GLYPH SHAPES (you probably don't need these, but they have to go before
// the bitmaps for C declaration-order reasons)
//

#ifndef STBTT_vmove // you can predefine these to use different values (but why?)
   enum {
      STBTT_vmove=1,
      STBTT_vline,
      STBTT_vcurve,
      STBTT_vcubic
   };
#endif

#ifndef stbtt_vertex // you can predefine this to use different values
                   // (we share this with other code at RAD)
   #define stbtt_vertex_type short // can't use stbtt_int16 because that's not visible in the header file
   typedef struct
   {
      stbtt_vertex_type x,y,cx,cy,cx1,cy1;
      unsigned char type,padding;
   } stbtt_vertex;
#endif

STBTT_DEF int stbtt_IsGlyphEmpty(const stbtt_fontinfo *info, int glyph_index);
// returns non-zero if nothing is drawn for this glyph

STBTT_DEF int stbtt_GetCodepointShape(const stbtt_fontinfo *info, int unicode_codepoint, stbtt_vertex **vertices);
STBTT_DEF int stbtt_GetGlyphShape(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **vertices);
// returns # of vertices and fills *vertices with the pointer to them
//   these are expressed in "unscaled" coordinates
//
// The shape is a series of contours. Each one starts with
// a STBTT_moveto, then consists of a series of mixed
// STBTT_lineto and STBTT_curveto segments. A lineto
// draws a line from previous endpoint to its x,y; a curveto
// draws a quadratic bezier from previous endpoint to
// its x,y, using cx,cy as the bezier control point.

STBTT_DEF void stbtt_FreeShape(const stbtt_fontinfo *info, stbtt_vertex *vertices);
// frees the data allocated above

STBTT_DEF unsigned char *stbtt_FindSVGDoc(const stbtt_fontinfo *info, int gl);
STBTT_DEF int stbtt_GetCodepointSVG(const stbtt_fontinfo *info, int unicode_codepoint, const char **svg);
STBTT_DEF int stbtt_GetGlyphSVG(const stbtt_fontinfo *info, int gl, const char **svg);
// fills svg with the character's SVG data.
// returns data size or 0 if SVG not found.

//////////////////////////////////////////////////////////////////////////////
//
// BITMAP RENDERING
//

STBTT_DEF void stbtt_FreeBitmap(unsigned char *bitmap, void *userdata);
// frees the bitmap allocated below

STBTT_DEF unsigned char *stbtt_GetCodepointBitmap(const stbtt_fontinfo *info, float scale_x, float scale_y, int codepoint, int *width, int *height, int *xoff, int *yoff);
// allocates a large-enough single-channel 8bpp bitmap and renders the
// specified character/glyph at the specified scale into it, with
// antialiasing. 0 is no coverage (transparent), 255 is fully covered (opaque).
// *width & *height are filled out with the width & height of the bitmap,
// which is stored left-to-right, top-to-bottom.
//
// xoff/yoff are the offset it pixel space from the glyph origin to the top-left of the bitmap

STBTT_DEF unsigned char *stbtt_GetCodepointBitmapSubpixel(const stbtt_fontinfo *info, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint, int *width, int *height, int *xoff, int *yoff);
// the same as stbtt_GetCodepoitnBitmap, but you can specify a subpixel
// shift for the character

STBTT_DEF void stbtt_MakeCodepointBitmap(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int codepoint);
// the same as stbtt_GetCodepointBitmap, but you pass in storage for the bitmap
// in the form of 'output', with row spacing of 'out_stride' bytes. the bitmap
// is clipped to out_w/out_h bytes. Call stbtt_GetCodepointBitmapBox to get the
// width and height and positioning info for it first.

STBTT_DEF void stbtt_MakeCodepointBitmapSubpixel(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint);
// same as stbtt_MakeCodepointBitmap, but you can specify a subpixel
// shift for the character

STBTT_DEF void stbtt_MakeCodepointBitmapSubpixelPrefilter(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int oversample_x, int oversample_y, float *sub_x, float *sub_y, int codepoint);
// same as stbtt_MakeCodepointBitmapSubpixel, but prefiltering
// is performed (see stbtt_PackSetOversampling)

STBTT_DEF void stbtt_GetCodepointBitmapBox(const stbtt_fontinfo *font, int codepoint, float scale_x, float scale_y, int *ix0, int *iy0, int *ix1, int *iy1);
// get the bbox of the bitmap centered around the glyph origin; so the
// bitmap width is ix1-ix0, height is iy1-iy0, and location to place
// the bitmap top left is (leftSideBearing*scale,iy0).
// (Note that the bitmap uses y-increases-down, but the shape uses
// y-increases-up, so CodepointBitmapBox and CodepointBox are inverted.)

STBTT_DEF void stbtt_GetCodepointBitmapBoxSubpixel(const stbtt_fontinfo *font, int codepoint, float scale_x, float scale_y, float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1);
// same as stbtt_GetCodepointBitmapBox, but you can specify a subpixel
// shift for the character

// the following functions are equivalent to the above functions, but operate
// on glyph indices instead of Unicode codepoints (for efficiency)
STBTT_DEF unsigned char *stbtt_GetGlyphBitmap(const stbtt_fontinfo *info, float scale_x, float scale_y, int glyph, int *width, int *height, int *xoff, int *yoff);
STBTT_DEF unsigned char *stbtt_GetGlyphBitmapSubpixel(const stbtt_fontinfo *info, float scale_x, float scale_y, float shift_x, float shift_y, int glyph, int *width, int *height, int *xoff, int *yoff);
STBTT_DEF void stbtt_MakeGlyphBitmap(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int glyph);
STBTT_DEF void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph);
STBTT_DEF void stbtt_MakeGlyphBitmapSubpixelPrefilter(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int oversample_x, int oversample_y, float *sub_x, float *sub_y, int glyph);
STBTT_DEF void stbtt_GetGlyphBitmapBox(const stbtt_fontinfo *font, int glyph, float scale_x, float scale_y, int *ix0, int *iy0, int *ix1, int *iy1);
STBTT_DEF void stbtt_GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo *font, int glyph, float scale_x, float scale_y,float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1);


// @TODO: don't expose this structure
typedef struct
{
   int w,h,stride;
   unsigned char *pixels;
} stbtt__bitmap;

// rasterize a shape with quadratic beziers into a bitmap
STBTT_DEF void stbtt_Rasterize(stbtt__bitmap *result,        // 1-channel bitmap to draw into
                               float flatness_in_pixels,     // allowable error of curve in pixels
                               stbtt_vertex *vertices,       // array of vertices defining shape
                               int num_verts,                // number of vertices in above array
                               float scale_x, float scale_y, // scale applied to input vertices
                               float shift_x, float shift_y, // translation applied to input vertices
                               int x_off, int y_off,         // another translation applied to input
                               int invert,                   // if non-zero, vertically flip shape
                               void *userdata);              // context for to STBTT_MALLOC

//////////////////////////////////////////////////////////////////////////////
//
// Signed Distance Function (or Field) rendering

STBTT_DEF void stbtt_FreeSDF(unsigned char *bitmap, void *userdata);
// frees the SDF bitmap allocated below

STBTT_DEF unsigned char * stbtt_GetGlyphSDF(const stbtt_fontinfo *info, float scale, int glyph, int padding, unsigned char onedge_value, float pixel_dist_scale, int *width, int *height, int *xoff, int *yoff);
STBTT_DEF unsigned char * stbtt_GetCodepointSDF(const stbtt_fontinfo *info, float scale, int codepoint, int padding, unsigned char onedge_value, float pixel_dist_scale, int *width, int *height, int *xoff, int *yoff);
// These functions compute a discretized SDF field for a single character, suitable for storing
// in a single-channel texture, sampling with bilinear filtering, and testing against
// larger than some threshold to produce scalable fonts.
//        info              --  the font
//        scale             --  controls the size of the resulting SDF bitmap, same as it would be creating a regular bitmap
//        glyph/codepoint   --  the character to generate the SDF for
//        padding           --  extra "pixels" around the character which are filled with the distance to the character (not 0),
//                                 which allows effects like bit outlines
//        onedge_value      --  value 0-255 to test the SDF against to reconstruct the character (i.e. the isocontour of the character)
//        pixel_dist_scale  --  what value the SDF should increase by when moving one SDF "pixel" away from the edge (on the 0..255 scale)
//                                 if positive, > onedge_value is inside; if negative, < onedge_value is inside
//        width,height      --  output height & width of the SDF bitmap (including padding)
//        xoff,yoff         --  output origin of the character
//        return value      --  a 2D array of bytes 0..255, width*height in size
//
// pixel_dist_scale & onedge_value are a scale & bias that allows you to make
// optimal use of the limited 0..255 for your application, trading off precision
// and special effects. SDF values outside the range 0..255 are clamped to 0..255.
//
// Example:
//      scale = stbtt_ScaleForPixelHeight(22)
//      padding = 5
//      onedge_value = 180
//      pixel_dist_scale = 180/5.0 = 36.0
//
//      This will create an SDF bitmap in which the character is about 22 pixels
//      high but the whole bitmap is about 22+5+5=32 pixels high. To produce a filled
//      shape, sample the SDF at each pixel and fill the pixel if the SDF value
//      is greater than or equal to 180/255. (You'll actually want to antialias,
//      which is beyond the scope of this example.) Additionally, you can compute
//      offset outlines (e.g. to stroke the character border inside & outside,
//      or only outside). For example, to fill outside the character up to 3 SDF
//      pixels, you would compare against (180-36.0*3)/255 = 72/255. The above
//      choice of variables maps a range from 5 pixels outside the shape to
//      2 pixels inside the shape to 0..255; this is intended primarily for apply
//      outside effects only (the interior range is needed to allow proper
//      antialiasing of the font at *smaller* sizes)
//
// The function computes the SDF analytically at each SDF pixel, not by e.g.
// building a higher-res bitmap and approximating it. In theory the quality
// should be as high as possible for an SDF of this size & representation, but
// unclear if this is true in practice (perhaps building a higher-res bitmap
// and computing from that can allow drop-out prevention).
//
// The algorithm has not been optimized at all, so expect it to be slow
// if computing lots of characters or very large sizes.



//////////////////////////////////////////////////////////////////////////////
//
// Finding the right font...
//
// You should really just solve this offline, keep your own tables
// of what font is what, and don't try to get it out of the .ttf file.
// That's because getting it out of the .ttf file is really hard, because
// the names in the file can appear in many possible encodings, in many
// possible languages, and e.g. if you need a case-insensitive comparison,
// the details of that depend on the encoding & language in a complex way
// (actually underspecified in truetype, but also gigantic).
//
// But you can use the provided functions in two possible ways:
//     stbtt_FindMatchingFont() will use *case-sensitive* comparisons on
//             unicode-encoded names to try to find the font you want;
//             you can run this before calling stbtt_InitFont()
//
//     stbtt_GetFontNameString() lets you get any of the various strings
//             from the file yourself and do your own comparisons on them.
//             You have to have called stbtt_InitFont() first.


STBTT_DEF int stbtt_FindMatchingFont(const unsigned char *fontdata, const char *name, int flags);
// returns the offset (not index) of the font that matches, or -1 if none
//   if you use STBTT_MACSTYLE_DONTCARE, use a font name like "Arial Bold".
//   if you use any other flag, use a font name like "Arial"; this checks
//     the 'macStyle' header field; i don't know if fonts set this consistently
#define STBTT_MACSTYLE_DONTCARE     0
#define STBTT_MACSTYLE_BOLD         1
#define STBTT_MACSTYLE_ITALIC       2
#define STBTT_MACSTYLE_UNDERSCORE   4
#define STBTT_MACSTYLE_NONE         8   // <= not same as 0, this makes us check the bitfield is 0

STBTT_DEF int stbtt_CompareUTF8toUTF16_bigendian(const char *s1, int len1, const char *s2, int len2);
// returns 1/0 whether the first string interpreted as utf8 is identical to
// the second string interpreted as big-endian utf16... useful for strings from next func

STBTT_DEF const char *stbtt_GetFontNameString(const stbtt_fontinfo *font, int *length, int platformID, int encodingID, int languageID, int nameID);
// returns the string (which may be big-endian double byte, e.g. for unicode)
// and puts the length in bytes in *length.
//
// some of the values for the IDs are below; for more see the truetype spec:
//     http://developer.apple.com/textfonts/TTRefMan/RM06/Chap6name.html
//     http://www.microsoft.com/typography/otspec/name.htm

enum { // platformID
   STBTT_PLATFORM_ID_UNICODE   =0,
   STBTT_PLATFORM_ID_MAC       =1,
   STBTT_PLATFORM_ID_ISO       =2,
   STBTT_PLATFORM_ID_MICROSOFT =3
};

enum { // encodingID for STBTT_PLATFORM_ID_UNICODE
   STBTT_UNICODE_EID_UNICODE_1_0    =0,
   STBTT_UNICODE_EID_UNICODE_1_1    =1,
   STBTT_UNICODE_EID_ISO_10646      =2,
   STBTT_UNICODE_EID_UNICODE_2_0_BMP=3,
   STBTT_UNICODE_EID_UNICODE_2_0_FULL=4
};

enum { // encodingID for STBTT_PLATFORM_ID_MICROSOFT
   STBTT_MS_EID_SYMBOL        =0,
   STBTT_MS_EID_UNICODE_BMP   =1,
   STBTT_MS_EID_SHIFTJIS      =2,
   STBTT_MS_EID_UNICODE_FULL  =10
};

enum { // encodingID for STBTT_PLATFORM_ID_MAC; same as Script Manager codes
   STBTT_MAC_EID_ROMAN        =0,   STBTT_MAC_EID_ARABIC       =4,
   STBTT_MAC_EID_JAPANESE     =1,   STBTT_MAC_EID_HEBREW       =5,
   STBTT_MAC_EID_CHINESE_TRAD =2,   STBTT_MAC_EID_GREEK        =6,
   STBTT_MAC_EID_KOREAN       =3,   STBTT_MAC_EID_RUSSIAN      =7
};

enum { // languageID for STBTT_PLATFORM_ID_MICROSOFT; same as LCID...
       // problematic because there are e.g. 16 english LCIDs and 16 arabic LCIDs
   STBTT_MS_LANG_ENGLISH     =0x0409,   STBTT_MS_LANG_ITALIAN     =0x0410,
   STBTT_MS_LANG_CHINESE     =0x0804,   STBTT_MS_LANG_JAPANESE    =0x0411,
   STBTT_MS_LANG_DUTCH       =0x0413,   STBTT_MS_LANG_KOREAN      =0x0412,
   STBTT_MS_LANG_FRENCH      =0x040c,   STBTT_MS_LANG_RUSSIAN     =0x0419,
   STBTT_MS_LANG_GERMAN      =0x0407,   STBTT_MS_LANG_SPANISH     =0x0409,
   STBTT_MS_LANG_HEBREW      =0x040d,   STBTT_MS_LANG_SWEDISH     =0x041D
};

enum { // languageID for STBTT_PLATFORM_ID_MAC
   STBTT_MAC_LANG_ENGLISH      =0 ,   STBTT_MAC_LANG_JAPANESE     =11,
   STBTT_MAC_LANG_ARABIC       =12,   STBTT_MAC_LANG_KOREAN       =23,
   STBTT_MAC_LANG_DUTCH        =4 ,   STBTT_MAC_LANG_RUSSIAN      =32,
   STBTT_MAC_LANG_FRENCH       =1 ,   STBTT_MAC_LANG_SPANISH      =6 ,
   STBTT_MAC_LANG_GERMAN       =2 ,   STBTT_MAC_LANG_SWEDISH      =5 ,
   STBTT_MAC_LANG_HEBREW       =10,   STBTT_MAC_LANG_CHINESE_SIMPLIFIED =33,
   STBTT_MAC_LANG_ITALIAN      =3 ,   STBTT_MAC_LANG_CHINESE_TRAD =19
};

#ifdef __cplusplus
}
#endif

#endif // __STB_INCLUDE_STB_TRUETYPE_H__

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////   IMPLEMENTATION
////
////

#ifdef STB_TRUETYPE_IMPLEMENTATION

#ifndef STBTT_MAX_OVERSAMPLE
#define STBTT_MAX_OVERSAMPLE   8
#endif

#if STBTT_MAX_OVERSAMPLE > 255
#error "STBTT_MAX_OVERSAMPLE cannot be > 255"
#endif

typedef int stbtt__test_oversample_pow2[(STBTT_MAX_OVERSAMPLE & (STBTT_MAX_OVERSAMPLE-1)) == 0 ? 1 : -1];

#ifndef STBTT_RASTERIZER_VERSION
#define STBTT_RASTERIZER_VERSION 2
#endif

#ifdef _MSC_VER
#define STBTT__NOTUSED(v)  (void)(v)
#else
#define STBTT__NOTUSED(v)  (void)sizeof(v)
#endif

//////////////////////////////////////////////////////////////////////////
//
// stbtt__buf helpers to parse data from file
//

static stbtt_uint8 stbtt__buf_get8(stbtt__buf *b)
{
   if (b->cursor >= b->size)
      return 0;
   return b->data[b->cursor++];
}

static stbtt_uint8 stbtt__buf_peek8(stbtt__buf *b)
{
   if (b->cursor >= b->size)
      return 0;
   return b->data[b->cursor];
}

static void stbtt__buf_seek(stbtt__buf *b, int o)
{
   STBTT_assert(!(o > b->size || o < 0));
   b->cursor = (o > b->size || o < 0) ? b->size : o;
}

static void stbtt__buf_skip(stbtt__buf *b, int o)
{
   stbtt__buf_seek(b, b->cursor + o);
}

static stbtt_uint32 stbtt__buf_get(stbtt__buf *b, int n)
{
   stbtt_uint32 v = 0;
   int i;
   STBTT_assert(n >= 1 && n <= 4);
   for (i = 0; i < n; i++)
      v = (v << 8) | stbtt__buf_get8(b);
   return v;
}

static stbtt__buf stbtt__new_buf(const void *p, size_t size)
{
   stbtt__buf r;
   STBTT_assert(size < 0x40000000);
   r.data = (stbtt_uint8*) p;
   r.size = (int) size;
   r.cursor = 0;
   return r;
}

#define stbtt__buf_get16(b)  stbtt__buf_get((b), 2)
#define stbtt__buf_get32(b)  stbtt__buf_get((b), 4)

static stbtt__buf stbtt__buf_range(const stbtt__buf *b, int o, int s)
{
   stbtt__buf r = stbtt__new_buf(NULL, 0);
   if (o < 0 || s < 0 || o > b->size || s > b->size - o) return r;
   r.data = b->data + o;
   r.size = s;
   return r;
}

static stbtt__buf stbtt__cff_get_index(stbtt__buf *b)
{
   int count, start, offsize;
   start = b->cursor;
   count = stbtt__buf_get16(b);
   if (count) {
      offsize = stbtt__buf_get8(b);
      STBTT_assert(offsize >= 1 && offsize <= 4);
      stbtt__buf_skip(b, offsize * count);
      stbtt__buf_skip(b, stbtt__buf_get(b, offsize) - 1);
   }
   return stbtt__buf_range(b, start, b->cursor - start);
}

static stbtt_uint32 stbtt__cff_int(stbtt__buf *b)
{
   int b0 = stbtt__buf_get8(b);
   if (b0 >= 32 && b0 <= 246)       return b0 - 139;
   else if (b0 >= 247 && b0 <= 250) return (b0 - 247)*256 + stbtt__buf_get8(b) + 108;
   else if (b0 >= 251 && b0 <= 254) return -(b0 - 251)*256 - stbtt__buf_get8(b) - 108;
   else if (b0 == 28)               return stbtt__buf_get16(b);
   else if (b0 == 29)               return stbtt__buf_get32(b);
   STBTT_assert(0);
   return 0;
}

static void stbtt__cff_skip_operand(stbtt__buf *b) {
   int v, b0 = stbtt__buf_peek8(b);
   STBTT_assert(b0 >= 28);
   if (b0 == 30) {
      stbtt__buf_skip(b, 1);
      while (b->cursor < b->size) {
         v = stbtt__buf_get8(b);
         if ((v & 0xF) == 0xF || (v >> 4) == 0xF)
            break;
      }
   } else {
      stbtt__cff_int(b);
   }
}

static stbtt__buf stbtt__dict_get(stbtt__buf *b, int key)
{
   stbtt__buf_seek(b, 0);
   while (b->cursor < b->size) {
      int start = b->cursor, end, op;
      while (stbtt__buf_peek8(b) >= 28)
         stbtt__cff_skip_operand(b);
      end = b->cursor;
      op = stbtt__buf_get8(b);
      if (op == 12)  op = stbtt__buf_get8(b) | 0x100;
      if (op == key) return stbtt__buf_range(b, start, end-start);
   }
   return stbtt__buf_range(b, 0, 0);
}

static void stbtt__dict_get_ints(stbtt__buf *b, int key, int outcount, stbtt_uint32 *out)
{
   int i;
   stbtt__buf operands = stbtt__dict_get(b, key);
   for (i = 0; i < outcount && operands.cursor < operands.size; i++)
      out[i] = stbtt__cff_int(&operands);
}

static int stbtt__cff_index_count(stbtt__buf *b)
{
   stbtt__buf_seek(b, 0);
   return stbtt__buf_get16(b);
}

static stbtt__buf stbtt__cff_index_get(stbtt__buf b, int i)
{
   int count, offsize, start, end;
   stbtt__buf_seek(&b, 0);
   count = stbtt__buf_get16(&b);
   offsize = stbtt__buf_get8(&b);
   STBTT_assert(i >= 0 && i < count);
   STBTT_assert(offsize >= 1 && offsize <= 4);
   stbtt__buf_skip(&b, i*offsize);
   start = stbtt__buf_get(&b, offsize);
   end = stbtt__buf_get(&b, offsize);
   return stbtt__buf_range(&b, 2+(count+1)*offsize+start, end - start);
}

//////////////////////////////////////////////////////////////////////////
//
// accessors to parse data from file
//

// on platforms that don't allow misaligned reads, if we want to allow
// truetype fonts that aren't padded to alignment, define ALLOW_UNALIGNED_TRUETYPE

#define ttBYTE(p)     (* (stbtt_uint8 *) (p))
#define ttCHAR(p)     (* (stbtt_int8 *) (p))
#define ttFixed(p)    ttLONG(p)

static stbtt_uint16 ttUSHORT(stbtt_uint8 *p) { return p[0]*256 + p[1]; }
static stbtt_int16 ttSHORT(stbtt_uint8 *p)   { return p[0]*256 + p[1]; }
static stbtt_uint32 ttULONG(stbtt_uint8 *p)  { return (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3]; }
static stbtt_int32 ttLONG(stbtt_uint8 *p)    { return (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3]; }

#define stbtt_tag4(p,c0,c1,c2,c3) ((p)[0] == (c0) && (p)[1] == (c1) && (p)[2] == (c2) && (p)[3] == (c3))
#define stbtt_tag(p,str)           stbtt_tag4(p,str[0],str[1],str[2],str[3])

static int stbtt__isfont(stbtt_uint8 *font)
{
   // check the version number
   if (stbtt_tag4(font, '1',0,0,0))  return 1; // TrueType 1
   if (stbtt_tag(font, "typ1"))   return 1; // TrueType with type 1 font -- we don't support this!
   if (stbtt_tag(font, "OTTO"))   return 1; // OpenType with CFF
   if (stbtt_tag4(font, 0,1,0,0)) return 1; // OpenType 1.0
   if (stbtt_tag(font, "true"))   return 1; // Apple specification for TrueType fonts
   return 0;
}

// @OPTIMIZE: binary search
static stbtt_uint32 stbtt__find_table(stbtt_uint8 *data, stbtt_uint32 fontstart, const char *tag)
{
   stbtt_int32 num_tables = ttUSHORT(data+fontstart+4);
   stbtt_uint32 tabledir = fontstart + 12;
   stbtt_int32 i;
   for (i=0; i < num_tables; ++i) {
      stbtt_uint32 loc = tabledir + 16*i;
      if (stbtt_tag(data+loc+0, tag))
         return ttULONG(data+loc+8);
   }
   return 0;
}

static int stbtt_GetFontOffsetForIndex_internal(unsigned char *font_collection, int index)
{
   // if it's just a font, there's only one valid index
   if (stbtt__isfont(font_collection))
      return index == 0 ? 0 : -1;

   // check if it's a TTC
   if (stbtt_tag(font_collection, "ttcf")) {
      // version 1?
      if (ttULONG(font_collection+4) == 0x00010000 || ttULONG(font_collection+4) == 0x00020000) {
         stbtt_int32 n = ttLONG(font_collection+8);
         if (index >= n)
            return -1;
         return ttULONG(font_collection+12+index*4);
      }
   }
   return -1;
}

static int stbtt_GetNumberOfFonts_internal(unsigned char *font_collection)
{
   // if it's just a font, there's only one valid font
   if (stbtt__isfont(font_collection))
      return 1;

   // check if it's a TTC
   if (stbtt_tag(font_collection, "ttcf")) {
      // version 1?
      if (ttULONG(font_collection+4) == 0x00010000 || ttULONG(font_collection+4) == 0x00020000) {
         return ttLONG(font_collection+8);
      }
   }
   return 0;
}

static stbtt__buf stbtt__get_subrs(stbtt__buf cff, stbtt__buf fontdict)
{
   stbtt_uint32 subrsoff = 0, private_loc[2] = { 0, 0 };
   stbtt__buf pdict;
   stbtt__dict_get_ints(&fontdict, 18, 2, private_loc);
   if (!private_loc[1] || !private_loc[0]) return stbtt__new_buf(NULL, 0);
   pdict = stbtt__buf_range(&cff, private_loc[1], private_loc[0]);
   stbtt__dict_get_ints(&pdict, 19, 1, &subrsoff);
   if (!subrsoff) return stbtt__new_buf(NULL, 0);
   stbtt__buf_seek(&cff, private_loc[1]+subrsoff);
   return stbtt__cff_get_index(&cff);
}

// since most people won't use this, find this table the first time it's needed
static int stbtt__get_svg(stbtt_fontinfo *info)
{
   stbtt_uint32 t;
   if (info->svg < 0) {
      t = stbtt__find_table(info->data, info->fontstart, "SVG ");
      if (t) {
         stbtt_uint32 offset = ttULONG(info->data + t + 2);
         info->svg = t + offset;
      } else {
         info->svg = 0;
      }
   }
   return info->svg;
}

static int stbtt_InitFont_internal(stbtt_fontinfo *info, unsigned char *data, int fontstart)
{
   stbtt_uint32 cmap, t;
   stbtt_int32 i,numTables;

   info->data = data;
   info->fontstart = fontstart;
   info->cff = stbtt__new_buf(NULL, 0);

   cmap = stbtt__find_table(data, fontstart, "cmap");       // required
   info->loca = stbtt__find_table(data, fontstart, "loca"); // required
   info->head = stbtt__find_table(data, fontstart, "head"); // required
   info->glyf = stbtt__find_table(data, fontstart, "glyf"); // required
   info->hhea = stbtt__find_table(data, fontstart, "hhea"); // required
   info->hmtx = stbtt__find_table(data, fontstart, "hmtx"); // required
   info->kern = stbtt__find_table(data, fontstart, "kern"); // not required
   info->gpos = stbtt__find_table(data, fontstart, "GPOS"); // not required

   if (!cmap || !info->head || !info->hhea || !info->hmtx)
      return 0;
   if (info->glyf) {
      // required for truetype
      if (!info->loca) return 0;
   } else {
      // initialization for CFF / Type2 fonts (OTF)
      stbtt__buf b, topdict, topdictidx;
      stbtt_uint32 cstype = 2, charstrings = 0, fdarrayoff = 0, fdselectoff = 0;
      stbtt_uint32 cff;

      cff = stbtt__find_table(data, fontstart, "CFF ");
      if (!cff) return 0;

      info->fontdicts = stbtt__new_buf(NULL, 0);
      info->fdselect = stbtt__new_buf(NULL, 0);

      // @TODO this should use size from table (not 512MB)
      info->cff = stbtt__new_buf(data+cff, 512*1024*1024);
      b = info->cff;

      // read the header
      stbtt__buf_skip(&b, 2);
      stbtt__buf_seek(&b, stbtt__buf_get8(&b)); // hdrsize

      // @TODO the name INDEX could list multiple fonts,
      // but we just use the first one.
      stbtt__cff_get_index(&b);  // name INDEX
      topdictidx = stbtt__cff_get_index(&b);
      topdict = stbtt__cff_index_get(topdictidx, 0);
      stbtt__cff_get_index(&b);  // string INDEX
      info->gsubrs = stbtt__cff_get_index(&b);

      stbtt__dict_get_ints(&topdict, 17, 1, &charstrings);
      stbtt__dict_get_ints(&topdict, 0x100 | 6, 1, &cstype);
      stbtt__dict_get_ints(&topdict, 0x100 | 36, 1, &fdarrayoff);
      stbtt__dict_get_ints(&topdict, 0x100 | 37, 1, &fdselectoff);
      info->subrs = stbtt__get_subrs(b, topdict);

      // we only support Type 2 charstrings
      if (cstype != 2) return 0;
      if (charstrings == 0) return 0;

      if (fdarrayoff) {
         // looks like a CID font
         if (!fdselectoff) return 0;
         stbtt__buf_seek(&b, fdarrayoff);
         info->fontdicts = stbtt__cff_get_index(&b);
         info->fdselect = stbtt__buf_range(&b, fdselectoff, b.size-fdselectoff);
      }

      stbtt__buf_seek(&b, charstrings);
      info->charstrings = stbtt__cff_get_index(&b);
   }

   t = stbtt__find_table(data, fontstart, "maxp");
   if (t)
      info->numGlyphs = ttUSHORT(data+t+4);
   else
      info->numGlyphs = 0xffff;

   info->svg = -1;

   // find a cmap encoding table we understand *now* to avoid searching
   // later. (todo: could make this installable)
   // the same regardless of glyph.
   numTables = ttUSHORT(data + cmap + 2);
   info->index_map = 0;
   for (i=0; i < numTables; ++i) {
      stbtt_uint32 encoding_record = cmap + 4 + 8 * i;
      // find an encoding we understand:
      switch(ttUSHORT(data+encoding_record)) {
         case STBTT_PLATFORM_ID_MICROSOFT:
            switch (ttUSHORT(data+encoding_record+2)) {
               case STBTT_MS_EID_UNICODE_BMP:
               case STBTT_MS_EID_UNICODE_FULL:
                  // MS/Unicode
                  info->index_map = cmap + ttULONG(data+encoding_record+4);
                  break;
            }
            break;
        case STBTT_PLATFORM_ID_UNICODE:
            // Mac/iOS has these
            // all the encodingIDs are unicode, so we don't bother to check it
            info->index_map = cmap + ttULONG(data+encoding_record+4);
            break;
      }
   }
   if (info->index_map == 0)
      return 0;

   info->indexToLocFormat = ttUSHORT(data+info->head + 50);
   return 1;
}

STBTT_DEF int stbtt_FindGlyphIndex(const stbtt_fontinfo *info, int unicode_codepoint)
{
   stbtt_uint8 *data = info->data;
   stbtt_uint32 index_map = info->index_map;

   stbtt_uint16 format = ttUSHORT(data + index_map + 0);
   if (format == 0) { // apple byte encoding
      stbtt_int32 bytes = ttUSHORT(data + index_map + 2);
      if (unicode_codepoint < bytes-6)
         return ttBYTE(data + index_map + 6 + unicode_codepoint);
      return 0;
   } else if (format == 6) {
      stbtt_uint32 first = ttUSHORT(data + index_map + 6);
      stbtt_uint32 count = ttUSHORT(data + index_map + 8);
      if ((stbtt_uint32) unicode_codepoint >= first && (stbtt_uint32) unicode_codepoint < first+count)
         return ttUSHORT(data + index_map + 10 + (unicode_codepoint - first)*2);
      return 0;
   } else if (format == 2) {
      STBTT_assert(0); // @TODO: high-byte mapping for japanese/chinese/korean
      return 0;
   } else if (format == 4) { // standard mapping for windows fonts: binary search collection of ranges
      stbtt_uint16 segcount = ttUSHORT(data+index_map+6) >> 1;
      stbtt_uint16 searchRange = ttUSHORT(data+index_map+8) >> 1;
      stbtt_uint16 entrySelector = ttUSHORT(data+index_map+10);
      stbtt_uint16 rangeShift = ttUSHORT(data+index_map+12) >> 1;

      // do a binary search of the segments
      stbtt_uint32 endCount = index_map + 14;
      stbtt_uint32 search = endCount;

      if (unicode_codepoint > 0xffff)
         return 0;

      // they lie from endCount .. endCount + segCount
      // but searchRange is the nearest power of two, so...
      if (unicode_codepoint >= ttUSHORT(data + search + rangeShift*2))
         search += rangeShift*2;

      // now decrement to bias correctly to find smallest
      search -= 2;
      while (entrySelector) {
         stbtt_uint16 end;
         searchRange >>= 1;
         end = ttUSHORT(data + search + searchRange*2);
         if (unicode_codepoint > end)
            search += searchRange*2;
         --entrySelector;
      }
      search += 2;

      {
         stbtt_uint16 offset, start, last;
         stbtt_uint16 item = (stbtt_uint16) ((search - endCount) >> 1);

         start = ttUSHORT(data + index_map + 14 + segcount*2 + 2 + 2*item);
         last = ttUSHORT(data + endCount + 2*item);
         if (unicode_codepoint < start || unicode_codepoint > last)
            return 0;

         offset = ttUSHORT(data + index_map + 14 + segcount*6 + 2 + 2*item);
         if (offset == 0)
            return (stbtt_uint16) (unicode_codepoint + ttSHORT(data + index_map + 14 + segcount*4 + 2 + 2*item));

         return ttUSHORT(data + offset + (unicode_codepoint-start)*2 + index_map + 14 + segcount*6 + 2 + 2*item);
      }
   } else if (format == 12 || format == 13) {
      stbtt_uint32 ngroups = ttULONG(data+index_map+12);
      stbtt_int32 low,high;
      low = 0; high = (stbtt_int32)ngroups;
      // Binary search the right group.
      while (low < high) {
         stbtt_int32 mid = low + ((high-low) >> 1); // rounds down, so low <= mid < high
         stbtt_uint32 start_char = ttULONG(data+index_map+16+mid*12);
         stbtt_uint32 end_char = ttULONG(data+index_map+16+mid*12+4);
         if ((stbtt_uint32) unicode_codepoint < start_char)
            high = mid;
         else if ((stbtt_uint32) unicode_codepoint > end_char)
            low = mid+1;
         else {
            stbtt_uint32 start_glyph = ttULONG(data+index_map+16+mid*12+8);
            if (format == 12)
               return start_glyph + unicode_codepoint-start_char;
            else // format == 13
               return start_glyph;
         }
      }
      return 0; // not found
   }
   // @TODO
   STBTT_assert(0);
   return 0;
}

STBTT_DEF int stbtt_GetCodepointShape(const stbtt_fontinfo *info, int unicode_codepoint, stbtt_vertex **vertices)
{
   return stbtt_GetGlyphShape(info, stbtt_FindGlyphIndex(info, unicode_codepoint), vertices);
}

static void stbtt_setvertex(stbtt_vertex *v, stbtt_uint8 type, stbtt_int32 x, stbtt_int32 y, stbtt_int32 cx, stbtt_int32 cy)
{
   v->type = type;
   v->x = (stbtt_int16) x;
   v->y = (stbtt_int16) y;
   v->cx = (stbtt_int16) cx;
   v->cy = (stbtt_int16) cy;
}

static int stbtt__GetGlyfOffset(const stbtt_fontinfo *info, int glyph_index)
{
   int g1,g2;

   STBTT_assert(!info->cff.size);

   if (glyph_index >= info->numGlyphs) return -1; // glyph index out of range
   if (info->indexToLocFormat >= 2)    return -1; // unknown index->glyph map format

   if (info->indexToLocFormat == 0) {
      g1 = info->glyf + ttUSHORT(info->data + info->loca + glyph_index * 2) * 2;
      g2 = info->glyf + ttUSHORT(info->data + info->loca + glyph_index * 2 + 2) * 2;
   } else {
      g1 = info->glyf + ttULONG (info->data + info->loca + glyph_index * 4);
      g2 = info->glyf + ttULONG (info->data + info->loca + glyph_index * 4 + 4);
   }

   return g1==g2 ? -1 : g1; // if length is 0, return -1
}

static int stbtt__GetGlyphInfoT2(const stbtt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1);

STBTT_DEF int stbtt_GetGlyphBox(const stbtt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1)
{
   if (info->cff.size) {
      stbtt__GetGlyphInfoT2(info, glyph_index, x0, y0, x1, y1);
   } else {
      int g = stbtt__GetGlyfOffset(info, glyph_index);
      if (g < 0) return 0;

      if (x0) *x0 = ttSHORT(info->data + g + 2);
      if (y0) *y0 = ttSHORT(info->data + g + 4);
      if (x1) *x1 = ttSHORT(info->data + g + 6);
      if (y1) *y1 = ttSHORT(info->data + g + 8);
   }
   return 1;
}

STBTT_DEF int stbtt_GetCodepointBox(const stbtt_fontinfo *info, int codepoint, int *x0, int *y0, int *x1, int *y1)
{
   return stbtt_GetGlyphBox(info, stbtt_FindGlyphIndex(info,codepoint), x0,y0,x1,y1);
}

STBTT_DEF int stbtt_IsGlyphEmpty(const stbtt_fontinfo *info, int glyph_index)
{
   stbtt_int16 numberOfContours;
   int g;
   if (info->cff.size)
      return stbtt__GetGlyphInfoT2(info, glyph_index, NULL, NULL, NULL, NULL) == 0;
   g = stbtt__GetGlyfOffset(info, glyph_index);
   if (g < 0) return 1;
   numberOfContours = ttSHORT(info->data + g);
   return numberOfContours == 0;
}

static int stbtt__close_shape(stbtt_vertex *vertices, int num_vertices, int was_off, int start_off,
    stbtt_int32 sx, stbtt_int32 sy, stbtt_int32 scx, stbtt_int32 scy, stbtt_int32 cx, stbtt_int32 cy)
{
   if (start_off) {
      if (was_off)
         stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx+scx)>>1, (cy+scy)>>1, cx,cy);
      stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, sx,sy,scx,scy);
   } else {
      if (was_off)
         stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve,sx,sy,cx,cy);
      else
         stbtt_setvertex(&vertices[num_vertices++], STBTT_vline,sx,sy,0,0);
   }
   return num_vertices;
}

static int stbtt__GetGlyphShapeTT(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **pvertices)
{
   stbtt_int16 numberOfContours;
   stbtt_uint8 *endPtsOfContours;
   stbtt_uint8 *data = info->data;
   stbtt_vertex *vertices=0;
   int num_vertices=0;
   int g = stbtt__GetGlyfOffset(info, glyph_index);

   *pvertices = NULL;

   if (g < 0) return 0;

   numberOfContours = ttSHORT(data + g);

   if (numberOfContours > 0) {
      stbtt_uint8 flags=0,flagcount;
      stbtt_int32 ins, i,j=0,m,n, next_move, was_off=0, off, start_off=0;
      stbtt_int32 x,y,cx,cy,sx,sy, scx,scy;
      stbtt_uint8 *points;
      endPtsOfContours = (data + g + 10);
      ins = ttUSHORT(data + g + 10 + numberOfContours * 2);
      points = data + g + 10 + numberOfContours * 2 + 2 + ins;

      n = 1+ttUSHORT(endPtsOfContours + numberOfContours*2-2);

      m = n + 2*numberOfContours;  // a loose bound on how many vertices we might need
      vertices = (stbtt_vertex *) STBTT_malloc(m * sizeof(vertices[0]), info->userdata);
      if (vertices == 0)
         return 0;

      next_move = 0;
      flagcount=0;

      // in first pass, we load uninterpreted data into the allocated array
      // above, shifted to the end of the array so we won't overwrite it when
      // we create our final data starting from the front

      off = m - n; // starting offset for uninterpreted data, regardless of how m ends up being calculated

      // first load flags

      for (i=0; i < n; ++i) {
         if (flagcount == 0) {
            flags = *points++;
            if (flags & 8)
               flagcount = *points++;
         } else
            --flagcount;
         vertices[off+i].type = flags;
      }

      // now load x coordinates
      x=0;
      for (i=0; i < n; ++i) {
         flags = vertices[off+i].type;
         if (flags & 2) {
            stbtt_int16 dx = *points++;
            x += (flags & 16) ? dx : -dx; // ???
         } else {
            if (!(flags & 16)) {
               x = x + (stbtt_int16) (points[0]*256 + points[1]);
               points += 2;
            }
         }
         vertices[off+i].x = (stbtt_int16) x;
      }

      // now load y coordinates
      y=0;
      for (i=0; i < n; ++i) {
         flags = vertices[off+i].type;
         if (flags & 4) {
            stbtt_int16 dy = *points++;
            y += (flags & 32) ? dy : -dy; // ???
         } else {
            if (!(flags & 32)) {
               y = y + (stbtt_int16) (points[0]*256 + points[1]);
               points += 2;
            }
         }
         vertices[off+i].y = (stbtt_int16) y;
      }

      // now convert them to our format
      num_vertices=0;
      sx = sy = cx = cy = scx = scy = 0;
      for (i=0; i < n; ++i) {
         flags = vertices[off+i].type;
         x     = (stbtt_int16) vertices[off+i].x;
         y     = (stbtt_int16) vertices[off+i].y;

         if (next_move == i) {
            if (i != 0)
               num_vertices = stbtt__close_shape(vertices, num_vertices, was_off, start_off, sx,sy,scx,scy,cx,cy);

            // now start the new one
            start_off = !(flags & 1);
            if (start_off) {
               // if we start off with an off-curve point, then when we need to find a point on the curve
               // where we can start, and we need to save some state for when we wraparound.
               scx = x;
               scy = y;
               if (!(vertices[off+i+1].type & 1)) {
                  // next point is also a curve point, so interpolate an on-point curve
                  sx = (x + (stbtt_int32) vertices[off+i+1].x) >> 1;
                  sy = (y + (stbtt_int32) vertices[off+i+1].y) >> 1;
               } else {
                  // otherwise just use the next point as our start point
                  sx = (stbtt_int32) vertices[off+i+1].x;
                  sy = (stbtt_int32) vertices[off+i+1].y;
                  ++i; // we're using point i+1 as the starting point, so skip it
               }
            } else {
               sx = x;
               sy = y;
            }
            stbtt_setvertex(&vertices[num_vertices++], STBTT_vmove,sx,sy,0,0);
            was_off = 0;
            next_move = 1 + ttUSHORT(endPtsOfContours+j*2);
            ++j;
         } else {
            if (!(flags & 1)) { // if it's a curve
               if (was_off) // two off-curve control points in a row means interpolate an on-curve midpoint
                  stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx+x)>>1, (cy+y)>>1, cx, cy);
               cx = x;
               cy = y;
               was_off = 1;
            } else {
               if (was_off)
                  stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, x,y, cx, cy);
               else
                  stbtt_setvertex(&vertices[num_vertices++], STBTT_vline, x,y,0,0);
               was_off = 0;
            }
         }
      }
      num_vertices = stbtt__close_shape(vertices, num_vertices, was_off, start_off, sx,sy,scx,scy,cx,cy);
   } else if (numberOfContours < 0) {
      // Compound shapes.
      int more = 1;
      stbtt_uint8 *comp = data + g + 10;
      num_vertices = 0;
      vertices = 0;
      while (more) {
         stbtt_uint16 flags, gidx;
         int comp_num_verts = 0, i;
         stbtt_vertex *comp_verts = 0, *tmp = 0;
         float mtx[6] = {1,0,0,1,0,0}, m, n;

         flags = ttSHORT(comp); comp+=2;
         gidx = ttSHORT(comp); comp+=2;

         if (flags & 2) { // XY values
            if (flags & 1) { // shorts
               mtx[4] = ttSHORT(comp); comp+=2;
               mtx[5] = ttSHORT(comp); comp+=2;
            } else {
               mtx[4] = ttCHAR(comp); comp+=1;
               mtx[5] = ttCHAR(comp); comp+=1;
            }
         }
         else {
            // @TODO handle matching point
            STBTT_assert(0);
         }
         if (flags & (1<<3)) { // WE_HAVE_A_SCALE
            mtx[0] = mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[1] = mtx[2] = 0;
         } else if (flags & (1<<6)) { // WE_HAVE_AN_X_AND_YSCALE
            mtx[0] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[1] = mtx[2] = 0;
            mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
         } else if (flags & (1<<7)) { // WE_HAVE_A_TWO_BY_TWO
            mtx[0] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[1] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[2] = ttSHORT(comp)/16384.0f; comp+=2;
            mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
         }

         // Find transformation scales.
         m = (float) STBTT_sqrt(mtx[0]*mtx[0] + mtx[1]*mtx[1]);
         n = (float) STBTT_sqrt(mtx[2]*mtx[2] + mtx[3]*mtx[3]);

         // Get indexed glyph.
         comp_num_verts = stbtt_GetGlyphShape(info, gidx, &comp_verts);
         if (comp_num_verts > 0) {
            // Transform vertices.
            for (i = 0; i < comp_num_verts; ++i) {
               stbtt_vertex* v = &comp_verts[i];
               stbtt_vertex_type x,y;
               x=v->x; y=v->y;
               v->x = (stbtt_vertex_type)(m * (mtx[0]*x + mtx[2]*y + mtx[4]));
               v->y = (stbtt_vertex_type)(n * (mtx[1]*x + mtx[3]*y + mtx[5]));
               x=v->cx; y=v->cy;
               v->cx = (stbtt_vertex_type)(m * (mtx[0]*x + mtx[2]*y + mtx[4]));
               v->cy = (stbtt_vertex_type)(n * (mtx[1]*x + mtx[3]*y + mtx[5]));
            }
            // Append vertices.
            tmp = (stbtt_vertex*)STBTT_malloc((num_vertices+comp_num_verts)*sizeof(stbtt_vertex), info->userdata);
            if (!tmp) {
               if (vertices) STBTT_free(vertices, info->userdata);
               if (comp_verts) STBTT_free(comp_verts, info->userdata);
               return 0;
            }
            if (num_vertices > 0 && vertices) STBTT_memcpy(tmp, vertices, num_vertices*sizeof(stbtt_vertex));
            STBTT_memcpy(tmp+num_vertices, comp_verts, comp_num_verts*sizeof(stbtt_vertex));
            if (vertices) STBTT_free(vertices, info->userdata);
            vertices = tmp;
            STBTT_free(comp_verts, info->userdata);
            num_vertices += comp_num_verts;
         }
         // More components ?
         more = flags & (1<<5);
      }
   } else {
      // numberOfCounters == 0, do nothing
   }

   *pvertices = vertices;
   return num_vertices;
}

typedef struct
{
   int bounds;
   int started;
   float first_x, first_y;
   float x, y;
   stbtt_int32 min_x, max_x, min_y, max_y;

   stbtt_vertex *pvertices;
   int num_vertices;
} stbtt__csctx;

#define STBTT__CSCTX_INIT(bounds) {bounds,0, 0,0, 0,0, 0,0,0,0, NULL, 0}

static void stbtt__track_vertex(stbtt__csctx *c, stbtt_int32 x, stbtt_int32 y)
{
   if (x > c->max_x || !c->started) c->max_x = x;
   if (y > c->max_y || !c->started) c->max_y = y;
   if (x < c->min_x || !c->started) c->min_x = x;
   if (y < c->min_y || !c->started) c->min_y = y;
   c->started = 1;
}

static void stbtt__csctx_v(stbtt__csctx *c, stbtt_uint8 type, stbtt_int32 x, stbtt_int32 y, stbtt_int32 cx, stbtt_int32 cy, stbtt_int32 cx1, stbtt_int32 cy1)
{
   if (c->bounds) {
      stbtt__track_vertex(c, x, y);
      if (type == STBTT_vcubic) {
         stbtt__track_vertex(c, cx, cy);
         stbtt__track_vertex(c, cx1, cy1);
      }
   } else {
      stbtt_setvertex(&c->pvertices[c->num_vertices], type, x, y, cx, cy);
      c->pvertices[c->num_vertices].cx1 = (stbtt_int16) cx1;
      c->pvertices[c->num_vertices].cy1 = (stbtt_int16) cy1;
   }
   c->num_vertices++;
}

static void stbtt__csctx_close_shape(stbtt__csctx *ctx)
{
   if (ctx->first_x != ctx->x || ctx->first_y != ctx->y)
      stbtt__csctx_v(ctx, STBTT_vline, (int)ctx->first_x, (int)ctx->first_y, 0, 0, 0, 0);
}

static void stbtt__csctx_rmove_to(stbtt__csctx *ctx, float dx, float dy)
{
   stbtt__csctx_close_shape(ctx);
   ctx->first_x = ctx->x = ctx->x + dx;
   ctx->first_y = ctx->y = ctx->y + dy;
   stbtt__csctx_v(ctx, STBTT_vmove, (int)ctx->x, (int)ctx->y, 0, 0, 0, 0);
}

static void stbtt__csctx_rline_to(stbtt__csctx *ctx, float dx, float dy)
{
   ctx->x += dx;
   ctx->y += dy;
   stbtt__csctx_v(ctx, STBTT_vline, (int)ctx->x, (int)ctx->y, 0, 0, 0, 0);
}

static void stbtt__csctx_rccurve_to(stbtt__csctx *ctx, float dx1, float dy1, float dx2, float dy2, float dx3, float dy3)
{
   float cx1 = ctx->x + dx1;
   float cy1 = ctx->y + dy1;
   float cx2 = cx1 + dx2;
   float cy2 = cy1 + dy2;
   ctx->x = cx2 + dx3;
   ctx->y = cy2 + dy3;
   stbtt__csctx_v(ctx, STBTT_vcubic, (int)ctx->x, (int)ctx->y, (int)cx1, (int)cy1, (int)cx2, (int)cy2);
}

static stbtt__buf stbtt__get_subr(stbtt__buf idx, int n)
{
   int count = stbtt__cff_index_count(&idx);
   int bias = 107;
   if (count >= 33900)
      bias = 32768;
   else if (count >= 1240)
      bias = 1131;
   n += bias;
   if (n < 0 || n >= count)
      return stbtt__new_buf(NULL, 0);
   return stbtt__cff_index_get(idx, n);
}

static stbtt__buf stbtt__cid_get_glyph_subrs(const stbtt_fontinfo *info, int glyph_index)
{
   stbtt__buf fdselect = info->fdselect;
   int nranges, start, end, v, fmt, fdselector = -1, i;

   stbtt__buf_seek(&fdselect, 0);
   fmt = stbtt__buf_get8(&fdselect);
   if (fmt == 0) {
      // untested
      stbtt__buf_skip(&fdselect, glyph_index);
      fdselector = stbtt__buf_get8(&fdselect);
   } else if (fmt == 3) {
      nranges = stbtt__buf_get16(&fdselect);
      start = stbtt__buf_get16(&fdselect);
      for (i = 0; i < nranges; i++) {
         v = stbtt__buf_get8(&fdselect);
         end = stbtt__buf_get16(&fdselect);
         if (glyph_index >= start && glyph_index < end) {
            fdselector = v;
            break;
         }
         start = end;
      }
   }
   if (fdselector == -1) stbtt__new_buf(NULL, 0);
   return stbtt__get_subrs(info->cff, stbtt__cff_index_get(info->fontdicts, fdselector));
}

static int stbtt__run_charstring(const stbtt_fontinfo *info, int glyph_index, stbtt__csctx *c)
{
   int in_header = 1, maskbits = 0, subr_stack_height = 0, sp = 0, v, i, b0;
   int has_subrs = 0, clear_stack;
   float s[48];
   stbtt__buf subr_stack[10], subrs = info->subrs, b;
   float f;

#define STBTT__CSERR(s) (0)

   // this currently ignores the initial width value, which isn't needed if we have hmtx
   b = stbtt__cff_index_get(info->charstrings, glyph_index);
   while (b.cursor < b.size) {
      i = 0;
      clear_stack = 1;
      b0 = stbtt__buf_get8(&b);
      switch (b0) {
      // @TODO implement hinting
      case 0x13: // hintmask
      case 0x14: // cntrmask
         if (in_header)
            maskbits += (sp / 2); // implicit "vstem"
         in_header = 0;
         stbtt__buf_skip(&b, (maskbits + 7) / 8);
         break;

      case 0x01: // hstem
      case 0x03: // vstem
      case 0x12: // hstemhm
      case 0x17: // vstemhm
         maskbits += (sp / 2);
         break;

      case 0x15: // rmoveto
         in_header = 0;
         if (sp < 2) return STBTT__CSERR("rmoveto stack");
         stbtt__csctx_rmove_to(c, s[sp-2], s[sp-1]);
         break;
      case 0x04: // vmoveto
         in_header = 0;
         if (sp < 1) return STBTT__CSERR("vmoveto stack");
         stbtt__csctx_rmove_to(c, 0, s[sp-1]);
         break;
      case 0x16: // hmoveto
         in_header = 0;
         if (sp < 1) return STBTT__CSERR("hmoveto stack");
         stbtt__csctx_rmove_to(c, s[sp-1], 0);
         break;

      case 0x05: // rlineto
         if (sp < 2) return STBTT__CSERR("rlineto stack");
         for (; i + 1 < sp; i += 2)
            stbtt__csctx_rline_to(c, s[i], s[i+1]);
         break;

      // hlineto/vlineto and vhcurveto/hvcurveto alternate horizontal and vertical
      // starting from a different place.

      case 0x07: // vlineto
         if (sp < 1) return STBTT__CSERR("vlineto stack");
         goto vlineto;
      case 0x06: // hlineto
         if (sp < 1) return STBTT__CSERR("hlineto stack");
         for (;;) {
            if (i >= sp) break;
            stbtt__csctx_rline_to(c, s[i], 0);
            i++;
      vlineto:
            if (i >= sp) break;
            stbtt__csctx_rline_to(c, 0, s[i]);
            i++;
         }
         break;

      case 0x1F: // hvcurveto
         if (sp < 4) return STBTT__CSERR("hvcurveto stack");
         goto hvcurveto;
      case 0x1E: // vhcurveto
         if (sp < 4) return STBTT__CSERR("vhcurveto stack");
         for (;;) {
            if (i + 3 >= sp) break;
            stbtt__csctx_rccurve_to(c, 0, s[i], s[i+1], s[i+2], s[i+3], (sp - i == 5) ? s[i + 4] : 0.0f);
            i += 4;
      hvcurveto:
            if (i + 3 >= sp) break;
            stbtt__csctx_rccurve_to(c, s[i], 0, s[i+1], s[i+2], (sp - i == 5) ? s[i+4] : 0.0f, s[i+3]);
            i += 4;
         }
         break;

      case 0x08: // rrcurveto
         if (sp < 6) return STBTT__CSERR("rcurveline stack");
         for (; i + 5 < sp; i += 6)
            stbtt__csctx_rccurve_to(c, s[i], s[i+1], s[i+2], s[i+3], s[i+4], s[i+5]);
         break;

      case 0x18: // rcurveline
         if (sp < 8) return STBTT__CSERR("rcurveline stack");
         for (; i + 5 < sp - 2; i += 6)
            stbtt__csctx_rccurve_to(c, s[i], s[i+1], s[i+2], s[i+3], s[i+4], s[i+5]);
         if (i + 1 >= sp) return STBTT__CSERR("rcurveline stack");
         stbtt__csctx_rline_to(c, s[i], s[i+1]);
         break;

      case 0x19: // rlinecurve
         if (sp < 8) return STBTT__CSERR("rlinecurve stack");
         for (; i + 1 < sp - 6; i += 2)
            stbtt__csctx_rline_to(c, s[i], s[i+1]);
         if (i + 5 >= sp) return STBTT__CSERR("rlinecurve stack");
         stbtt__csctx_rccurve_to(c, s[i], s[i+1], s[i+2], s[i+3], s[i+4], s[i+5]);
         break;

      case 0x1A: // vvcurveto
      case 0x1B: // hhcurveto
         if (sp < 4) return STBTT__CSERR("(vv|hh)curveto stack");
         f = 0.0;
         if (sp & 1) { f = s[i]; i++; }
         for (; i + 3 < sp; i += 4) {
            if (b0 == 0x1B)
               stbtt__csctx_rccurve_to(c, s[i], f, s[i+1], s[i+2], s[i+3], 0.0);
            else
               stbtt__csctx_rccurve_to(c, f, s[i], s[i+1], s[i+2], 0.0, s[i+3]);
            f = 0.0;
         }
         break;

      case 0x0A: // callsubr
         if (!has_subrs) {
            if (info->fdselect.size)
               subrs = stbtt__cid_get_glyph_subrs(info, glyph_index);
            has_subrs = 1;
         }
         // FALLTHROUGH
      case 0x1D: // callgsubr
         if (sp < 1) return STBTT__CSERR("call(g|)subr stack");
         v = (int) s[--sp];
         if (subr_stack_height >= 10) return STBTT__CSERR("recursion limit");
         subr_stack[subr_stack_height++] = b;
         b = stbtt__get_subr(b0 == 0x0A ? subrs : info->gsubrs, v);
         if (b.size == 0) return STBTT__CSERR("subr not found");
         b.cursor = 0;
         clear_stack = 0;
         break;

      case 0x0B: // return
         if (subr_stack_height <= 0) return STBTT__CSERR("return outside subr");
         b = subr_stack[--subr_stack_height];
         clear_stack = 0;
         break;

      case 0x0E: // endchar
         stbtt__csctx_close_shape(c);
         return 1;

      case 0x0C: { // two-byte escape
         float dx1, dx2, dx3, dx4, dx5, dx6, dy1, dy2, dy3, dy4, dy5, dy6;
         float dx, dy;
         int b1 = stbtt__buf_get8(&b);
         switch (b1) {
         // @TODO These "flex" implementations ignore the flex-depth and resolution,
         // and always draw beziers.
         case 0x22: // hflex
            if (sp < 7) return STBTT__CSERR("hflex stack");
            dx1 = s[0];
            dx2 = s[1];
            dy2 = s[2];
            dx3 = s[3];
            dx4 = s[4];
            dx5 = s[5];
            dx6 = s[6];
            stbtt__csctx_rccurve_to(c, dx1, 0, dx2, dy2, dx3, 0);
            stbtt__csctx_rccurve_to(c, dx4, 0, dx5, -dy2, dx6, 0);
            break;

         case 0x23: // flex
            if (sp < 13) return STBTT__CSERR("flex stack");
            dx1 = s[0];
            dy1 = s[1];
            dx2 = s[2];
            dy2 = s[3];
            dx3 = s[4];
            dy3 = s[5];
            dx4 = s[6];
            dy4 = s[7];
            dx5 = s[8];
            dy5 = s[9];
            dx6 = s[10];
            dy6 = s[11];
            //fd is s[12]
            stbtt__csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, dy3);
            stbtt__csctx_rccurve_to(c, dx4, dy4, dx5, dy5, dx6, dy6);
            break;

         case 0x24: // hflex1
            if (sp < 9) return STBTT__CSERR("hflex1 stack");
            dx1 = s[0];
            dy1 = s[1];
            dx2 = s[2];
            dy2 = s[3];
            dx3 = s[4];
            dx4 = s[5];
            dx5 = s[6];
            dy5 = s[7];
            dx6 = s[8];
            stbtt__csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, 0);
            stbtt__csctx_rccurve_to(c, dx4, 0, dx5, dy5, dx6, -(dy1+dy2+dy5));
            break;

         case 0x25: // flex1
            if (sp < 11) return STBTT__CSERR("flex1 stack");
            dx1 = s[0];
            dy1 = s[1];
            dx2 = s[2];
            dy2 = s[3];
            dx3 = s[4];
            dy3 = s[5];
            dx4 = s[6];
            dy4 = s[7];
            dx5 = s[8];
            dy5 = s[9];
            dx6 = dy6 = s[10];
            dx = dx1+dx2+dx3+dx4+dx5;
            dy = dy1+dy2+dy3+dy4+dy5;
            if (STBTT_fabs(dx) > STBTT_fabs(dy))
               dy6 = -dy;
            else
               dx6 = -dx;
            stbtt__csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, dy3);
            stbtt__csctx_rccurve_to(c, dx4, dy4, dx5, dy5, dx6, dy6);
            break;

         default:
            return STBTT__CSERR("unimplemented");
         }
      } break;

      default:
         if (b0 != 255 && b0 != 28 && b0 < 32)
            return STBTT__CSERR("reserved operator");

         // push immediate
         if (b0 == 255) {
            f = (float)(stbtt_int32)stbtt__buf_get32(&b) / 0x10000;
         } else {
            stbtt__buf_skip(&b, -1);
            f = (float)(stbtt_int16)stbtt__cff_int(&b);
         }
         if (sp >= 48) return STBTT__CSERR("push stack overflow");
         s[sp++] = f;
         clear_stack = 0;
         break;
      }
      if (clear_stack) sp = 0;
   }
   return STBTT__CSERR("no endchar");

#undef STBTT__CSERR
}

static int stbtt__GetGlyphShapeT2(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **pvertices)
{
   // runs the charstring twice, once to count and once to output (to avoid realloc)
   stbtt__csctx count_ctx = STBTT__CSCTX_INIT(1);
   stbtt__csctx output_ctx = STBTT__CSCTX_INIT(0);
   if (stbtt__run_charstring(info, glyph_index, &count_ctx)) {
      *pvertices = (stbtt_vertex*)STBTT_malloc(count_ctx.num_vertices*sizeof(stbtt_vertex), info->userdata);
      output_ctx.pvertices = *pvertices;
      if (stbtt__run_charstring(info, glyph_index, &output_ctx)) {
         STBTT_assert(output_ctx.num_vertices == count_ctx.num_vertices);
         return output_ctx.num_vertices;
      }
   }
   *pvertices = NULL;
   return 0;
}

static int stbtt__GetGlyphInfoT2(const stbtt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1)
{
   stbtt__csctx c = STBTT__CSCTX_INIT(1);
   int r = stbtt__run_charstring(info, glyph_index, &c);
   if (x0)  *x0 = r ? c.min_x : 0;
   if (y0)  *y0 = r ? c.min_y : 0;
   if (x1)  *x1 = r ? c.max_x : 0;
   if (y1)  *y1 = r ? c.max_y : 0;
   return r ? c.num_vertices : 0;
}

STBTT_DEF int stbtt_GetGlyphShape(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **pvertices)
{
   if (!info->cff.size)
      return stbtt__GetGlyphShapeTT(info, glyph_index, pvertices);
   else
      return stbtt__GetGlyphShapeT2(info, glyph_index, pvertices);
}

STBTT_DEF void stbtt_GetGlyphHMetrics(const stbtt_fontinfo *info, int glyph_index, int *advanceWidth, int *leftSideBearing)
{
   stbtt_uint16 numOfLongHorMetrics = ttUSHORT(info->data+info->hhea + 34);
   if (glyph_index < numOfLongHorMetrics) {
      if (advanceWidth)     *advanceWidth    = ttSHORT(info->data + info->hmtx + 4*glyph_index);
      if (leftSideBearing)  *leftSideBearing = ttSHORT(info->data + info->hmtx + 4*glyph_index + 2);
   } else {
      if (advanceWidth)     *advanceWidth    = ttSHORT(info->data + info->hmtx + 4*(numOfLongHorMetrics-1));
      if (leftSideBearing)  *leftSideBearing = ttSHORT(info->data + info->hmtx + 4*numOfLongHorMetrics + 2*(glyph_index - numOfLongHorMetrics));
   }
}

STBTT_DEF int  stbtt_GetKerningTableLength(const stbtt_fontinfo *info)
{
   stbtt_uint8 *data = info->data + info->kern;

   // we only look at the first table. it must be 'horizontal' and format 0.
   if (!info->kern)
      return 0;
   if (ttUSHORT(data+2) < 1) // number of tables, need at least 1
      return 0;
   if (ttUSHORT(data+8) != 1) // horizontal flag must be set in format
      return 0;

   return ttUSHORT(data+10);
}

STBTT_DEF int stbtt_GetKerningTable(const stbtt_fontinfo *info, stbtt_kerningentry* table, int table_length)
{
   stbtt_uint8 *data = info->data + info->kern;
   int k, length;

   // we only look at the first table. it must be 'horizontal' and format 0.
   if (!info->kern)
      return 0;
   if (ttUSHORT(data+2) < 1) // number of tables, need at least 1
      return 0;
   if (ttUSHORT(data+8) != 1) // horizontal flag must be set in format
      return 0;

   length = ttUSHORT(data+10);
   if (table_length < length)
      length = table_length;

   for (k = 0; k < length; k++)
   {
      table[k].glyph1 = ttUSHORT(data+18+(k*6));
      table[k].glyph2 = ttUSHORT(data+20+(k*6));
      table[k].advance = ttSHORT(data+22+(k*6));
   }

   return length;
}

static int stbtt__GetGlyphKernInfoAdvance(const stbtt_fontinfo *info, int glyph1, int glyph2)
{
   stbtt_uint8 *data = info->data + info->kern;
   stbtt_uint32 needle, straw;
   int l, r, m;

   // we only look at the first table. it must be 'horizontal' and format 0.
   if (!info->kern)
      return 0;
   if (ttUSHORT(data+2) < 1) // number of tables, need at least 1
      return 0;
   if (ttUSHORT(data+8) != 1) // horizontal flag must be set in format
      return 0;

   l = 0;
   r = ttUSHORT(data+10) - 1;
   needle = glyph1 << 16 | glyph2;
   while (l <= r) {
      m = (l + r) >> 1;
      straw = ttULONG(data+18+(m*6)); // note: unaligned read
      if (needle < straw)
         r = m - 1;
      else if (needle > straw)
         l = m + 1;
      else
         return ttSHORT(data+22+(m*6));
   }
   return 0;
}

static stbtt_int32 stbtt__GetCoverageIndex(stbtt_uint8 *coverageTable, int glyph)
{
   stbtt_uint16 coverageFormat = ttUSHORT(coverageTable);
   switch (coverageFormat) {
      case 1: {
         stbtt_uint16 glyphCount = ttUSHORT(coverageTable + 2);

         // Binary search.
         stbtt_int32 l=0, r=glyphCount-1, m;
         int straw, needle=glyph;
         while (l <= r) {
            stbtt_uint8 *glyphArray = coverageTable + 4;
            stbtt_uint16 glyphID;
            m = (l + r) >> 1;
            glyphID = ttUSHORT(glyphArray + 2 * m);
            straw = glyphID;
            if (needle < straw)
               r = m - 1;
            else if (needle > straw)
               l = m + 1;
            else {
               return m;
            }
         }
         break;
      }

      case 2: {
         stbtt_uint16 rangeCount = ttUSHORT(coverageTable + 2);
         stbtt_uint8 *rangeArray = coverageTable + 4;

         // Binary search.
         stbtt_int32 l=0, r=rangeCount-1, m;
         int strawStart, strawEnd, needle=glyph;
         while (l <= r) {
            stbtt_uint8 *rangeRecord;
            m = (l + r) >> 1;
            rangeRecord = rangeArray + 6 * m;
            strawStart = ttUSHORT(rangeRecord);
            strawEnd = ttUSHORT(rangeRecord + 2);
            if (needle < strawStart)
               r = m - 1;
            else if (needle > strawEnd)
               l = m + 1;
            else {
               stbtt_uint16 startCoverageIndex = ttUSHORT(rangeRecord + 4);
               return startCoverageIndex + glyph - strawStart;
            }
         }
         break;
      }

      default: return -1; // unsupported
   }

   return -1;
}

static stbtt_int32  stbtt__GetGlyphClass(stbtt_uint8 *classDefTable, int glyph)
{
   stbtt_uint16 classDefFormat = ttUSHORT(classDefTable);
   switch (classDefFormat)
   {
      case 1: {
         stbtt_uint16 startGlyphID = ttUSHORT(classDefTable + 2);
         stbtt_uint16 glyphCount = ttUSHORT(classDefTable + 4);
         stbtt_uint8 *classDef1ValueArray = classDefTable + 6;

         if (glyph >= startGlyphID && glyph < startGlyphID + glyphCount)
            return (stbtt_int32)ttUSHORT(classDef1ValueArray + 2 * (glyph - startGlyphID));
         break;
      }

      case 2: {
         stbtt_uint16 classRangeCount = ttUSHORT(classDefTable + 2);
         stbtt_uint8 *classRangeRecords = classDefTable + 4;

         // Binary search.
         stbtt_int32 l=0, r=classRangeCount-1, m;
         int strawStart, strawEnd, needle=glyph;
         while (l <= r) {
            stbtt_uint8 *classRangeRecord;
            m = (l + r) >> 1;
            classRangeRecord = classRangeRecords + 6 * m;
            strawStart = ttUSHORT(classRangeRecord);
            strawEnd = ttUSHORT(classRangeRecord + 2);
            if (needle < strawStart)
               r = m - 1;
            else if (needle > strawEnd)
               l = m + 1;
            else
               return (stbtt_int32)ttUSHORT(classRangeRecord + 4);
         }
         break;
      }

      default:
         return -1; // Unsupported definition type, return an error.
   }

   // "All glyphs not assigned to a class fall into class 0". (OpenType spec)
   return 0;
}

// Define to STBTT_assert(x) if you want to break on unimplemented formats.
#define STBTT_GPOS_TODO_assert(x)

static stbtt_int32 stbtt__GetGlyphGPOSInfoAdvance(const stbtt_fontinfo *info, int glyph1, int glyph2)
{
   stbtt_uint16 lookupListOffset;
   stbtt_uint8 *lookupList;
   stbtt_uint16 lookupCount;
   stbtt_uint8 *data;
   stbtt_int32 i, sti;

   if (!info->gpos) return 0;

   data = info->data + info->gpos;

   if (ttUSHORT(data+0) != 1) return 0; // Major version 1
   if (ttUSHORT(data+2) != 0) return 0; // Minor version 0

   lookupListOffset = ttUSHORT(data+8);
   lookupList = data + lookupListOffset;
   lookupCount = ttUSHORT(lookupList);

   for (i=0; i<lookupCount; ++i) {
      stbtt_uint16 lookupOffset = ttUSHORT(lookupList + 2 + 2 * i);
      stbtt_uint8 *lookupTable = lookupList + lookupOffset;

      stbtt_uint16 lookupType = ttUSHORT(lookupTable);
      stbtt_uint16 subTableCount = ttUSHORT(lookupTable + 4);
      stbtt_uint8 *subTableOffsets = lookupTable + 6;
      if (lookupType != 2) // Pair Adjustment Positioning Subtable
         continue;

      for (sti=0; sti<subTableCount; sti++) {
         stbtt_uint16 subtableOffset = ttUSHORT(subTableOffsets + 2 * sti);
         stbtt_uint8 *table = lookupTable + subtableOffset;
         stbtt_uint16 posFormat = ttUSHORT(table);
         stbtt_uint16 coverageOffset = ttUSHORT(table + 2);
         stbtt_int32 coverageIndex = stbtt__GetCoverageIndex(table + coverageOffset, glyph1);
         if (coverageIndex == -1) continue;

         switch (posFormat) {
            case 1: {
               stbtt_int32 l, r, m;
               int straw, needle;
               stbtt_uint16 valueFormat1 = ttUSHORT(table + 4);
               stbtt_uint16 valueFormat2 = ttUSHORT(table + 6);
               if (valueFormat1 == 4 && valueFormat2 == 0) { // Support more formats?
                  stbtt_int32 valueRecordPairSizeInBytes = 2;
                  stbtt_uint16 pairSetCount = ttUSHORT(table + 8);
                  stbtt_uint16 pairPosOffset = ttUSHORT(table + 10 + 2 * coverageIndex);
                  stbtt_uint8 *pairValueTable = table + pairPosOffset;
                  stbtt_uint16 pairValueCount = ttUSHORT(pairValueTable);
                  stbtt_uint8 *pairValueArray = pairValueTable + 2;

                  if (coverageIndex >= pairSetCount) return 0;

                  needle=glyph2;
                  r=pairValueCount-1;
                  l=0;

                  // Binary search.
                  while (l <= r) {
                     stbtt_uint16 secondGlyph;
                     stbtt_uint8 *pairValue;
                     m = (l + r) >> 1;
                     pairValue = pairValueArray + (2 + valueRecordPairSizeInBytes) * m;
                     secondGlyph = ttUSHORT(pairValue);
                     straw = secondGlyph;
                     if (needle < straw)
                        r = m - 1;
                     else if (needle > straw)
                        l = m + 1;
                     else {
                        stbtt_int16 xAdvance = ttSHORT(pairValue + 2);
                        return xAdvance;
                     }
                  }
               } else
                  return 0;
               break;
            }

            case 2: {
               stbtt_uint16 valueFormat1 = ttUSHORT(table + 4);
               stbtt_uint16 valueFormat2 = ttUSHORT(table + 6);
               if (valueFormat1 == 4 && valueFormat2 == 0) { // Support more formats?
                  stbtt_uint16 classDef1Offset = ttUSHORT(table + 8);
                  stbtt_uint16 classDef2Offset = ttUSHORT(table + 10);
                  int glyph1class = stbtt__GetGlyphClass(table + classDef1Offset, glyph1);
                  int glyph2class = stbtt__GetGlyphClass(table + classDef2Offset, glyph2);

                  stbtt_uint16 class1Count = ttUSHORT(table + 12);
                  stbtt_uint16 class2Count = ttUSHORT(table + 14);
                  stbtt_uint8 *class1Records, *class2Records;
                  stbtt_int16 xAdvance;

                  if (glyph1class < 0 || glyph1class >= class1Count) return 0; // malformed
                  if (glyph2class < 0 || glyph2class >= class2Count) return 0; // malformed

                  class1Records = table + 16;
                  class2Records = class1Records + 2 * (glyph1class * class2Count);
                  xAdvance = ttSHORT(class2Records + 2 * glyph2class);
                  return xAdvance;
               } else
                  return 0;
               break;
            }

            default:
               return 0; // Unsupported position format
         }
      }
   }

   return 0;
}

STBTT_DEF int  stbtt_GetGlyphKernAdvance(const stbtt_fontinfo *info, int g1, int g2)
{
   int xAdvance = 0;

   if (info->gpos)
      xAdvance += stbtt__GetGlyphGPOSInfoAdvance(info, g1, g2);
   else if (info->kern)
      xAdvance += stbtt__GetGlyphKernInfoAdvance(info, g1, g2);

   return xAdvance;
}

STBTT_DEF int  stbtt_GetCodepointKernAdvance(const stbtt_fontinfo *info, int ch1, int ch2)
{
   if (!info->kern && !info->gpos) // if no kerning table, don't waste time looking up both codepoint->glyphs
      return 0;
   return stbtt_GetGlyphKernAdvance(info, stbtt_FindGlyphIndex(info,ch1), stbtt_FindGlyphIndex(info,ch2));
}

STBTT_DEF void stbtt_GetCodepointHMetrics(const stbtt_fontinfo *info, int codepoint, int *advanceWidth, int *leftSideBearing)
{
   stbtt_GetGlyphHMetrics(info, stbtt_FindGlyphIndex(info,codepoint), advanceWidth, leftSideBearing);
}

STBTT_DEF void stbtt_GetFontVMetrics(const stbtt_fontinfo *info, int *ascent, int *descent, int *lineGap)
{
   if (ascent ) *ascent  = ttSHORT(info->data+info->hhea + 4);
   if (descent) *descent = ttSHORT(info->data+info->hhea + 6);
   if (lineGap) *lineGap = ttSHORT(info->data+info->hhea + 8);
}

STBTT_DEF int  stbtt_GetFontVMetricsOS2(const stbtt_fontinfo *info, int *typoAscent, int *typoDescent, int *typoLineGap)
{
   int tab = stbtt__find_table(info->data, info->fontstart, "OS/2");
   if (!tab)
      return 0;
   if (typoAscent ) *typoAscent  = ttSHORT(info->data+tab + 68);
   if (typoDescent) *typoDescent = ttSHORT(info->data+tab + 70);
   if (typoLineGap) *typoLineGap = ttSHORT(info->data+tab + 72);
   return 1;
}

STBTT_DEF void stbtt_GetFontBoundingBox(const stbtt_fontinfo *info, int *x0, int *y0, int *x1, int *y1)
{
   *x0 = ttSHORT(info->data + info->head + 36);
   *y0 = ttSHORT(info->data + info->head + 38);
   *x1 = ttSHORT(info->data + info->head + 40);
   *y1 = ttSHORT(info->data + info->head + 42);
}

STBTT_DEF float stbtt_ScaleForPixelHeight(const stbtt_fontinfo *info, float height)
{
   int fheight = ttSHORT(info->data + info->hhea + 4) - ttSHORT(info->data + info->hhea + 6);
   return (float) height / fheight;
}

STBTT_DEF float stbtt_ScaleForMappingEmToPixels(const stbtt_fontinfo *info, float pixels)
{
   int unitsPerEm = ttUSHORT(info->data + info->head + 18);
   return pixels / unitsPerEm;
}

STBTT_DEF void stbtt_FreeShape(const stbtt_fontinfo *info, stbtt_vertex *v)
{
   STBTT_free(v, info->userdata);
}

STBTT_DEF stbtt_uint8 *stbtt_FindSVGDoc(const stbtt_fontinfo *info, int gl)
{
   int i;
   stbtt_uint8 *data = info->data;
   stbtt_uint8 *svg_doc_list = data + stbtt__get_svg((stbtt_fontinfo *) info);

   int numEntries = ttUSHORT(svg_doc_list);
   stbtt_uint8 *svg_docs = svg_doc_list + 2;

   for(i=0; i<numEntries; i++) {
      stbtt_uint8 *svg_doc = svg_docs + (12 * i);
      if ((gl >= ttUSHORT(svg_doc)) && (gl <= ttUSHORT(svg_doc + 2)))
         return svg_doc;
   }
   return 0;
}

STBTT_DEF int stbtt_GetGlyphSVG(const stbtt_fontinfo *info, int gl, const char **svg)
{
   stbtt_uint8 *data = info->data;
   stbtt_uint8 *svg_doc;

   if (info->svg == 0)
      return 0;

   svg_doc = stbtt_FindSVGDoc(info, gl);
   if (svg_doc != NULL) {
      *svg = (char *) data + info->svg + ttULONG(svg_doc + 4);
      return ttULONG(svg_doc + 8);
   } else {
      return 0;
   }
}

STBTT_DEF int stbtt_GetCodepointSVG(const stbtt_fontinfo *info, int unicode_codepoint, const char **svg)
{
   return stbtt_GetGlyphSVG(info, stbtt_FindGlyphIndex(info, unicode_codepoint), svg);
}

//////////////////////////////////////////////////////////////////////////////
//
// antialiasing software rasterizer
//

STBTT_DEF void stbtt_GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo *font, int glyph, float scale_x, float scale_y,float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1)
{
   int x0=0,y0=0,x1,y1; // =0 suppresses compiler warning
   if (!stbtt_GetGlyphBox(font, glyph, &x0,&y0,&x1,&y1)) {
      // e.g. space character
      if (ix0) *ix0 = 0;
      if (iy0) *iy0 = 0;
      if (ix1) *ix1 = 0;
      if (iy1) *iy1 = 0;
   } else {
      // move to integral bboxes (treating pixels as little squares, what pixels get touched)?
      if (ix0) *ix0 = STBTT_ifloor( x0 * scale_x + shift_x);
      if (iy0) *iy0 = STBTT_ifloor(-y1 * scale_y + shift_y);
      if (ix1) *ix1 = STBTT_iceil ( x1 * scale_x + shift_x);
      if (iy1) *iy1 = STBTT_iceil (-y0 * scale_y + shift_y);
   }
}

STBTT_DEF void stbtt_GetGlyphBitmapBox(const stbtt_fontinfo *font, int glyph, float scale_x, float scale_y, int *ix0, int *iy0, int *ix1, int *iy1)
{
   stbtt_GetGlyphBitmapBoxSubpixel(font, glyph, scale_x, scale_y,0.0f,0.0f, ix0, iy0, ix1, iy1);
}

STBTT_DEF void stbtt_GetCodepointBitmapBoxSubpixel(const stbtt_fontinfo *font, int codepoint, float scale_x, float scale_y, float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1)
{
   stbtt_GetGlyphBitmapBoxSubpixel(font, stbtt_FindGlyphIndex(font,codepoint), scale_x, scale_y,shift_x,shift_y, ix0,iy0,ix1,iy1);
}

STBTT_DEF void stbtt_GetCodepointBitmapBox(const stbtt_fontinfo *font, int codepoint, float scale_x, float scale_y, int *ix0, int *iy0, int *ix1, int *iy1)
{
   stbtt_GetCodepointBitmapBoxSubpixel(font, codepoint, scale_x, scale_y,0.0f,0.0f, ix0,iy0,ix1,iy1);
}

//////////////////////////////////////////////////////////////////////////////
//
//  Rasterizer

typedef struct stbtt__hheap_chunk
{
   struct stbtt__hheap_chunk *next;
} stbtt__hheap_chunk;

typedef struct stbtt__hheap
{
   struct stbtt__hheap_chunk *head;
   void   *first_free;
   int    num_remaining_in_head_chunk;
} stbtt__hheap;

static void *stbtt__hheap_alloc(stbtt__hheap *hh, size_t size, void *userdata)
{
   if (hh->first_free) {
      void *p = hh->first_free;
      hh->first_free = * (void **) p;
      return p;
   } else {
      if (hh->num_remaining_in_head_chunk == 0) {
         int count = (size < 32 ? 2000 : size < 128 ? 800 : 100);
         stbtt__hheap_chunk *c = (stbtt__hheap_chunk *) STBTT_malloc(sizeof(stbtt__hheap_chunk) + size * count, userdata);
         if (c == NULL)
            return NULL;
         c->next = hh->head;
         hh->head = c;
         hh->num_remaining_in_head_chunk = count;
      }
      --hh->num_remaining_in_head_chunk;
      return (char *) (hh->head) + sizeof(stbtt__hheap_chunk) + size * hh->num_remaining_in_head_chunk;
   }
}

static void stbtt__hheap_free(stbtt__hheap *hh, void *p)
{
   *(void **) p = hh->first_free;
   hh->first_free = p;
}

static void stbtt__hheap_cleanup(stbtt__hheap *hh, void *userdata)
{
   stbtt__hheap_chunk *c = hh->head;
   while (c) {
      stbtt__hheap_chunk *n = c->next;
      STBTT_free(c, userdata);
      c = n;
   }
}

typedef struct stbtt__edge {
   float x0,y0, x1,y1;
   int invert;
} stbtt__edge;


typedef struct stbtt__active_edge
{
   struct stbtt__active_edge *next;
   #if STBTT_RASTERIZER_VERSION==1
   int x,dx;
   float ey;
   int direction;
   #elif STBTT_RASTERIZER_VERSION==2
   float fx,fdx,fdy;
   float direction;
   float sy;
   float ey;
   #else
   #error "Unrecognized value of STBTT_RASTERIZER_VERSION"
   #endif
} stbtt__active_edge;

#if STBTT_RASTERIZER_VERSION == 1
#define STBTT_FIXSHIFT   10
#define STBTT_FIX        (1 << STBTT_FIXSHIFT)
#define STBTT_FIXMASK    (STBTT_FIX-1)

static stbtt__active_edge *stbtt__new_active(stbtt__hheap *hh, stbtt__edge *e, int off_x, float start_point, void *userdata)
{
   stbtt__active_edge *z = (stbtt__active_edge *) stbtt__hheap_alloc(hh, sizeof(*z), userdata);
   float dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
   STBTT_assert(z != NULL);
   if (!z) return z;

   // round dx down to avoid overshooting
   if (dxdy < 0)
      z->dx = -STBTT_ifloor(STBTT_FIX * -dxdy);
   else
      z->dx = STBTT_ifloor(STBTT_FIX * dxdy);

   z->x = STBTT_ifloor(STBTT_FIX * e->x0 + z->dx * (start_point - e->y0)); // use z->dx so when we offset later it's by the same amount
   z->x -= off_x * STBTT_FIX;

   z->ey = e->y1;
   z->next = 0;
   z->direction = e->invert ? 1 : -1;
   return z;
}
#elif STBTT_RASTERIZER_VERSION == 2
static stbtt__active_edge *stbtt__new_active(stbtt__hheap *hh, stbtt__edge *e, int off_x, float start_point, void *userdata)
{
   stbtt__active_edge *z = (stbtt__active_edge *) stbtt__hheap_alloc(hh, sizeof(*z), userdata);
   float dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
   STBTT_assert(z != NULL);
   //STBTT_assert(e->y0 <= start_point);
   if (!z) return z;
   z->fdx = dxdy;
   z->fdy = dxdy != 0.0f ? (1.0f/dxdy) : 0.0f;
   z->fx = e->x0 + dxdy * (start_point - e->y0);
   z->fx -= off_x;
   z->direction = e->invert ? 1.0f : -1.0f;
   z->sy = e->y0;
   z->ey = e->y1;
   z->next = 0;
   return z;
}
#else
#error "Unrecognized value of STBTT_RASTERIZER_VERSION"
#endif

#if STBTT_RASTERIZER_VERSION == 1
// note: this routine clips fills that extend off the edges... ideally this
// wouldn't happen, but it could happen if the truetype glyph bounding boxes
// are wrong, or if the user supplies a too-small bitmap
static void stbtt__fill_active_edges(unsigned char *scanline, int len, stbtt__active_edge *e, int max_weight)
{
   // non-zero winding fill
   int x0=0, w=0;

   while (e) {
      if (w == 0) {
         // if we're currently at zero, we need to record the edge start point
         x0 = e->x; w += e->direction;
      } else {
         int x1 = e->x; w += e->direction;
         // if we went to zero, we need to draw
         if (w == 0) {
            int i = x0 >> STBTT_FIXSHIFT;
            int j = x1 >> STBTT_FIXSHIFT;

            if (i < len && j >= 0) {
               if (i == j) {
                  // x0,x1 are the same pixel, so compute combined coverage
                  scanline[i] = scanline[i] + (stbtt_uint8) ((x1 - x0) * max_weight >> STBTT_FIXSHIFT);
               } else {
                  if (i >= 0) // add antialiasing for x0
                     scanline[i] = scanline[i] + (stbtt_uint8) (((STBTT_FIX - (x0 & STBTT_FIXMASK)) * max_weight) >> STBTT_FIXSHIFT);
                  else
                     i = -1; // clip

                  if (j < len) // add antialiasing for x1
                     scanline[j] = scanline[j] + (stbtt_uint8) (((x1 & STBTT_FIXMASK) * max_weight) >> STBTT_FIXSHIFT);
                  else
                     j = len; // clip

                  for (++i; i < j; ++i) // fill pixels between x0 and x1
                     scanline[i] = scanline[i] + (stbtt_uint8) max_weight;
               }
            }
         }
      }

      e = e->next;
   }
}

static void stbtt__rasterize_sorted_edges(stbtt__bitmap *result, stbtt__edge *e, int n, int vsubsample, int off_x, int off_y, void *userdata)
{
   stbtt__hheap hh = { 0, 0, 0 };
   stbtt__active_edge *active = NULL;
   int y,j=0;
   int max_weight = (255 / vsubsample);  // weight per vertical scanline
   int s; // vertical subsample index
   unsigned char scanline_data[512], *scanline;

   if (result->w > 512)
      scanline = (unsigned char *) STBTT_malloc(result->w, userdata);
   else
      scanline = scanline_data;

   y = off_y * vsubsample;
   e[n].y0 = (off_y + result->h) * (float) vsubsample + 1;

   while (j < result->h) {
      STBTT_memset(scanline, 0, result->w);
      for (s=0; s < vsubsample; ++s) {
         // find center of pixel for this scanline
         float scan_y = y + 0.5f;
         stbtt__active_edge **step = &active;

         // update all active edges;
         // remove all active edges that terminate before the center of this scanline
         while (*step) {
            stbtt__active_edge * z = *step;
            if (z->ey <= scan_y) {
               *step = z->next; // delete from list
               STBTT_assert(z->direction);
               z->direction = 0;
               stbtt__hheap_free(&hh, z);
            } else {
               z->x += z->dx; // advance to position for current scanline
               step = &((*step)->next); // advance through list
            }
         }

         // resort the list if needed
         for(;;) {
            int changed=0;
            step = &active;
            while (*step && (*step)->next) {
               if ((*step)->x > (*step)->next->x) {
                  stbtt__active_edge *t = *step;
                  stbtt__active_edge *q = t->next;

                  t->next = q->next;
                  q->next = t;
                  *step = q;
                  changed = 1;
               }
               step = &(*step)->next;
            }
            if (!changed) break;
         }

         // insert all edges that start before the center of this scanline -- omit ones that also end on this scanline
         while (e->y0 <= scan_y) {
            if (e->y1 > scan_y) {
               stbtt__active_edge *z = stbtt__new_active(&hh, e, off_x, scan_y, userdata);
               if (z != NULL) {
                  // find insertion point
                  if (active == NULL)
                     active = z;
                  else if (z->x < active->x) {
                     // insert at front
                     z->next = active;
                     active = z;
                  } else {
                     // find thing to insert AFTER
                     stbtt__active_edge *p = active;
                     while (p->next && p->next->x < z->x)
                        p = p->next;
                     // at this point, p->next->x is NOT < z->x
                     z->next = p->next;
                     p->next = z;
                  }
               }
            }
            ++e;
         }

         // now process all active edges in XOR fashion
         if (active)
            stbtt__fill_active_edges(scanline, result->w, active, max_weight);

         ++y;
      }
      STBTT_memcpy(result->pixels + j * result->stride, scanline, result->w);
      ++j;
   }

   stbtt__hheap_cleanup(&hh, userdata);

   if (scanline != scanline_data)
      STBTT_free(scanline, userdata);
}

#elif STBTT_RASTERIZER_VERSION == 2

// the edge passed in here does not cross the vertical line at x or the vertical line at x+1
// (i.e. it has already been clipped to those)
static void stbtt__handle_clipped_edge(float *scanline, int x, stbtt__active_edge *e, float x0, float y0, float x1, float y1)
{
   if (y0 == y1) return;
   STBTT_assert(y0 < y1);
   STBTT_assert(e->sy <= e->ey);
   if (y0 > e->ey) return;
   if (y1 < e->sy) return;
   if (y0 < e->sy) {
      x0 += (x1-x0) * (e->sy - y0) / (y1-y0);
      y0 = e->sy;
   }
   if (y1 > e->ey) {
      x1 += (x1-x0) * (e->ey - y1) / (y1-y0);
      y1 = e->ey;
   }

   if (x0 == x)
      STBTT_assert(x1 <= x+1);
   else if (x0 == x+1)
      STBTT_assert(x1 >= x);
   else if (x0 <= x)
      STBTT_assert(x1 <= x);
   else if (x0 >= x+1)
      STBTT_assert(x1 >= x+1);
   else
      STBTT_assert(x1 >= x && x1 <= x+1);

   if (x0 <= x && x1 <= x)
      scanline[x] += e->direction * (y1-y0);
   else if (x0 >= x+1 && x1 >= x+1)
      ;
   else {
      STBTT_assert(x0 >= x && x0 <= x+1 && x1 >= x && x1 <= x+1);
      scanline[x] += e->direction * (y1-y0) * (1-((x0-x)+(x1-x))/2); // coverage = 1 - average x position
   }
}

static float stbtt__sized_trapezoid_area(float height, float top_width, float bottom_width)
{
   STBTT_assert(top_width >= 0);
   STBTT_assert(bottom_width >= 0);
   return (top_width + bottom_width) / 2.0f * height;
}

static float stbtt__position_trapezoid_area(float height, float tx0, float tx1, float bx0, float bx1)
{
   return stbtt__sized_trapezoid_area(height, tx1 - tx0, bx1 - bx0);
}

static float stbtt__sized_triangle_area(float height, float width)
{
   return height * width / 2;
}

static void stbtt__fill_active_edges_new(float *scanline, float *scanline_fill, int len, stbtt__active_edge *e, float y_top)
{
   float y_bottom = y_top+1;

   while (e) {
      // brute force every pixel

      // compute intersection points with top & bottom
      STBTT_assert(e->ey >= y_top);

      if (e->fdx == 0) {
         float x0 = e->fx;
         if (x0 < len) {
            if (x0 >= 0) {
               stbtt__handle_clipped_edge(scanline,(int) x0,e, x0,y_top, x0,y_bottom);
               stbtt__handle_clipped_edge(scanline_fill-1,(int) x0+1,e, x0,y_top, x0,y_bottom);
            } else {
               stbtt__handle_clipped_edge(scanline_fill-1,0,e, x0,y_top, x0,y_bottom);
            }
         }
      } else {
         float x0 = e->fx;
         float dx = e->fdx;
         float xb = x0 + dx;
         float x_top, x_bottom;
         float sy0,sy1;
         float dy = e->fdy;
         STBTT_assert(e->sy <= y_bottom && e->ey >= y_top);

         // compute endpoints of line segment clipped to this scanline (if the
         // line segment starts on this scanline. x0 is the intersection of the
         // line with y_top, but that may be off the line segment.
         if (e->sy > y_top) {
            x_top = x0 + dx * (e->sy - y_top);
            sy0 = e->sy;
         } else {
            x_top = x0;
            sy0 = y_top;
         }
         if (e->ey < y_bottom) {
            x_bottom = x0 + dx * (e->ey - y_top);
            sy1 = e->ey;
         } else {
            x_bottom = xb;
            sy1 = y_bottom;
         }

         if (x_top >= 0 && x_bottom >= 0 && x_top < len && x_bottom < len) {
            // from here on, we don't have to range check x values

            if ((int) x_top == (int) x_bottom) {
               float height;
               // simple case, only spans one pixel
               int x = (int) x_top;
               height = (sy1 - sy0) * e->direction;
               STBTT_assert(x >= 0 && x < len);
               scanline[x]      += stbtt__position_trapezoid_area(height, x_top, x+1.0f, x_bottom, x+1.0f);
               scanline_fill[x] += height; // everything right of this pixel is filled
            } else {
               int x,x1,x2;
               float y_crossing, y_final, step, sign, area;
               // covers 2+ pixels
               if (x_top > x_bottom) {
                  // flip scanline vertically; signed area is the same
                  float t;
                  sy0 = y_bottom - (sy0 - y_top);
                  sy1 = y_bottom - (sy1 - y_top);
                  t = sy0, sy0 = sy1, sy1 = t;
                  t = x_bottom, x_bottom = x_top, x_top = t;
                  dx = -dx;
                  dy = -dy;
                  t = x0, x0 = xb, xb = t;
               }
               STBTT_assert(dy >= 0);
               STBTT_assert(dx >= 0);

               x1 = (int) x_top;
               x2 = (int) x_bottom;
               // compute intersection with y axis at x1+1
               y_crossing = y_top + dy * (x1+1 - x0);

               // compute intersection with y axis at x2
               y_final = y_top + dy * (x2 - x0);

               //           x1    x_top                            x2    x_bottom
               //     y_top  +------|-----+------------+------------+--------|---+------------+
               //            |            |            |            |            |            |
               //            |            |            |            |            |            |
               //       sy0  |      Txxxxx|............|............|............|............|
               // y_crossing |            *xxxxx.......|............|............|............|
               //            |            |     xxxxx..|............|............|............|
               //            |            |     /-   xx*xxxx........|............|............|
               //            |            | dy <       |    xxxxxx..|............|............|
               //   y_final  |            |     \-     |          xx*xxx.........|............|
               //       sy1  |            |            |            |   xxxxxB...|............|
               //            |            |            |            |            |            |
               //            |            |            |            |            |            |
               //  y_bottom  +------------+------------+------------+------------+------------+
               //
               // goal is to measure the area covered by '.' in each pixel

               // if x2 is right at the right edge of x1, y_crossing can blow up, github #1057
               // @TODO: maybe test against sy1 rather than y_bottom?
               if (y_crossing > y_bottom)
                  y_crossing = y_bottom;

               sign = e->direction;

               // area of the rectangle covered from sy0..y_crossing
               area = sign * (y_crossing-sy0);

               // area of the triangle (x_top,sy0), (x1+1,sy0), (x1+1,y_crossing)
               scanline[x1] += stbtt__sized_triangle_area(area, x1+1 - x_top);

               // check if final y_crossing is blown up; no test case for this
               if (y_final > y_bottom) {
                  y_final = y_bottom;
                  dy = (y_final - y_crossing ) / (x2 - (x1+1)); // if denom=0, y_final = y_crossing, so y_final <= y_bottom
               }

               // in second pixel, area covered by line segment found in first pixel
               // is always a rectangle 1 wide * the height of that line segment; this
               // is exactly what the variable 'area' stores. it also gets a contribution
               // from the line segment within it. the THIRD pixel will get the first
               // pixel's rectangle contribution, the second pixel's rectangle contribution,
               // and its own contribution. the 'own contribution' is the same in every pixel except
               // the leftmost and rightmost, a trapezoid that slides down in each pixel.
               // the second pixel's contribution to the third pixel will be the
               // rectangle 1 wide times the height change in the second pixel, which is dy.

               step = sign * dy * 1; // dy is dy/dx, change in y for every 1 change in x,
               // which multiplied by 1-pixel-width is how much pixel area changes for each step in x
               // so the area advances by 'step' every time

               for (x = x1+1; x < x2; ++x) {
                  scanline[x] += area + step/2; // area of trapezoid is 1*step/2
                  area += step;
               }
               STBTT_assert(STBTT_fabs(area) <= 1.01f); // accumulated error from area += step unless we round step down
               STBTT_assert(sy1 > y_final-0.01f);

               // area covered in the last pixel is the rectangle from all the pixels to the left,
               // plus the trapezoid filled by the line segment in this pixel all the way to the right edge
               scanline[x2] += area + sign * stbtt__position_trapezoid_area(sy1-y_final, (float) x2, x2+1.0f, x_bottom, x2+1.0f);

               // the rest of the line is filled based on the total height of the line segment in this pixel
               scanline_fill[x2] += sign * (sy1-sy0);
            }
         } else {
            // if edge goes outside of box we're drawing, we require
            // clipping logic. since this does not match the intended use
            // of this library, we use a different, very slow brute
            // force implementation
            // note though that this does happen some of the time because
            // x_top and x_bottom can be extrapolated at the top & bottom of
            // the shape and actually lie outside the bounding box
            int x;
            for (x=0; x < len; ++x) {
               // cases:
               //
               // there can be up to two intersections with the pixel. any intersection
               // with left or right edges can be handled by splitting into two (or three)
               // regions. intersections with top & bottom do not necessitate case-wise logic.
               //
               // the old way of doing this found the intersections with the left & right edges,
               // then used some simple logic to produce up to three segments in sorted order
               // from top-to-bottom. however, this had a problem: if an x edge was epsilon
               // across the x border, then the corresponding y position might not be distinct
               // from the other y segment, and it might ignored as an empty segment. to avoid
               // that, we need to explicitly produce segments based on x positions.

               // rename variables to clearly-defined pairs
               float y0 = y_top;
               float x1 = (float) (x);
               float x2 = (float) (x+1);
               float x3 = xb;
               float y3 = y_bottom;

               // x = e->x + e->dx * (y-y_top)
               // (y-y_top) = (x - e->x) / e->dx
               // y = (x - e->x) / e->dx + y_top
               float y1 = (x - x0) / dx + y_top;
               float y2 = (x+1 - x0) / dx + y_top;

               if (x0 < x1 && x3 > x2) {         // three segments descending down-right
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x1,y1);
                  stbtt__handle_clipped_edge(scanline,x,e, x1,y1, x2,y2);
                  stbtt__handle_clipped_edge(scanline,x,e, x2,y2, x3,y3);
               } else if (x3 < x1 && x0 > x2) {  // three segments descending down-left
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x2,y2);
                  stbtt__handle_clipped_edge(scanline,x,e, x2,y2, x1,y1);
                  stbtt__handle_clipped_edge(scanline,x,e, x1,y1, x3,y3);
               } else if (x0 < x1 && x3 > x1) {  // two segments across x, down-right
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x1,y1);
                  stbtt__handle_clipped_edge(scanline,x,e, x1,y1, x3,y3);
               } else if (x3 < x1 && x0 > x1) {  // two segments across x, down-left
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x1,y1);
                  stbtt__handle_clipped_edge(scanline,x,e, x1,y1, x3,y3);
               } else if (x0 < x2 && x3 > x2) {  // two segments across x+1, down-right
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x2,y2);
                  stbtt__handle_clipped_edge(scanline,x,e, x2,y2, x3,y3);
               } else if (x3 < x2 && x0 > x2) {  // two segments across x+1, down-left
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x2,y2);
                  stbtt__handle_clipped_edge(scanline,x,e, x2,y2, x3,y3);
               } else {  // one segment
                  stbtt__handle_clipped_edge(scanline,x,e, x0,y0, x3,y3);
               }
            }
         }
      }
      e = e->next;
   }
}

// directly AA rasterize edges w/o supersampling
static void stbtt__rasterize_sorted_edges(stbtt__bitmap *result, stbtt__edge *e, int n, int vsubsample, int off_x, int off_y, void *userdata)
{
   stbtt__hheap hh = { 0, 0, 0 };
   stbtt__active_edge *active = NULL;
   int y,j=0, i;
   float scanline_data[129], *scanline, *scanline2;

   STBTT__NOTUSED(vsubsample);

   if (result->w > 64)
      scanline = (float *) STBTT_malloc((result->w*2+1) * sizeof(float), userdata);
   else
      scanline = scanline_data;

   scanline2 = scanline + result->w;

   y = off_y;
   e[n].y0 = (float) (off_y + result->h) + 1;

   while (j < result->h) {
      // find center of pixel for this scanline
      float scan_y_top    = y + 0.0f;
      float scan_y_bottom = y + 1.0f;
      stbtt__active_edge **step = &active;

      STBTT_memset(scanline , 0, result->w*sizeof(scanline[0]));
      STBTT_memset(scanline2, 0, (result->w+1)*sizeof(scanline[0]));

      // update all active edges;
      // remove all active edges that terminate before the top of this scanline
      while (*step) {
         stbtt__active_edge * z = *step;
         if (z->ey <= scan_y_top) {
            *step = z->next; // delete from list
            STBTT_assert(z->direction);
            z->direction = 0;
            stbtt__hheap_free(&hh, z);
         } else {
            step = &((*step)->next); // advance through list
         }
      }

      // insert all edges that start before the bottom of this scanline
      while (e->y0 <= scan_y_bottom) {
         if (e->y0 != e->y1) {
            stbtt__active_edge *z = stbtt__new_active(&hh, e, off_x, scan_y_top, userdata);
            if (z != NULL) {
               if (j == 0 && off_y != 0) {
                  if (z->ey < scan_y_top) {
                     // this can happen due to subpixel positioning and some kind of fp rounding error i think
                     z->ey = scan_y_top;
                  }
               }
               STBTT_assert(z->ey >= scan_y_top); // if we get really unlucky a tiny bit of an edge can be out of bounds
               // insert at front
               z->next = active;
               active = z;
            }
         }
         ++e;
      }

      // now process all active edges
      if (active)
         stbtt__fill_active_edges_new(scanline, scanline2+1, result->w, active, scan_y_top);

      {
         float sum = 0;
         for (i=0; i < result->w; ++i) {
            float k;
            int m;
            sum += scanline2[i];
            k = scanline[i] + sum;
            k = (float) STBTT_fabs(k)*255 + 0.5f;
            m = (int) k;
            if (m > 255) m = 255;
            result->pixels[j*result->stride + i] = (unsigned char) m;
         }
      }
      // advance all the edges
      step = &active;
      while (*step) {
         stbtt__active_edge *z = *step;
         z->fx += z->fdx; // advance to position for current scanline
         step = &((*step)->next); // advance through list
      }

      ++y;
      ++j;
   }

   stbtt__hheap_cleanup(&hh, userdata);

   if (scanline != scanline_data)
      STBTT_free(scanline, userdata);
}
#else
#error "Unrecognized value of STBTT_RASTERIZER_VERSION"
#endif

#define STBTT__COMPARE(a,b)  ((a)->y0 < (b)->y0)

static void stbtt__sort_edges_ins_sort(stbtt__edge *p, int n)
{
   int i,j;
   for (i=1; i < n; ++i) {
      stbtt__edge t = p[i], *a = &t;
      j = i;
      while (j > 0) {
         stbtt__edge *b = &p[j-1];
         int c = STBTT__COMPARE(a,b);
         if (!c) break;
         p[j] = p[j-1];
         --j;
      }
      if (i != j)
         p[j] = t;
   }
}

static void stbtt__sort_edges_quicksort(stbtt__edge *p, int n)
{
   /* threshold for transitioning to insertion sort */
   while (n > 12) {
      stbtt__edge t;
      int c01,c12,c,m,i,j;

      /* compute median of three */
      m = n >> 1;
      c01 = STBTT__COMPARE(&p[0],&p[m]);
      c12 = STBTT__COMPARE(&p[m],&p[n-1]);
      /* if 0 >= mid >= end, or 0 < mid < end, then use mid */
      if (c01 != c12) {
         /* otherwise, we'll need to swap something else to middle */
         int z;
         c = STBTT__COMPARE(&p[0],&p[n-1]);
         /* 0>mid && mid<n:  0>n => n; 0<n => 0 */
         /* 0<mid && mid>n:  0>n => 0; 0<n => n */
         z = (c == c12) ? 0 : n-1;
         t = p[z];
         p[z] = p[m];
         p[m] = t;
      }
      /* now p[m] is the median-of-three */
      /* swap it to the beginning so it won't move around */
      t = p[0];
      p[0] = p[m];
      p[m] = t;

      /* partition loop */
      i=1;
      j=n-1;
      for(;;) {
         /* handling of equality is crucial here */
         /* for sentinels & efficiency with duplicates */
         for (;;++i) {
            if (!STBTT__COMPARE(&p[i], &p[0])) break;
         }
         for (;;--j) {
            if (!STBTT__COMPARE(&p[0], &p[j])) break;
         }
         /* make sure we haven't crossed */
         if (i >= j) break;
         t = p[i];
         p[i] = p[j];
         p[j] = t;

         ++i;
         --j;
      }
      /* recurse on smaller side, iterate on larger */
      if (j < (n-i)) {
         stbtt__sort_edges_quicksort(p,j);
         p = p+i;
         n = n-i;
      } else {
         stbtt__sort_edges_quicksort(p+i, n-i);
         n = j;
      }
   }
}

static void stbtt__sort_edges(stbtt__edge *p, int n)
{
   stbtt__sort_edges_quicksort(p, n);
   stbtt__sort_edges_ins_sort(p, n);
}

typedef struct
{
   float x,y;
} stbtt__point;

static void stbtt__rasterize(stbtt__bitmap *result, stbtt__point *pts, int *wcount, int windings, float scale_x, float scale_y, float shift_x, float shift_y, int off_x, int off_y, int invert, void *userdata)
{
   float y_scale_inv = invert ? -scale_y : scale_y;
   stbtt__edge *e;
   int n,i,j,k,m;
#if STBTT_RASTERIZER_VERSION == 1
   int vsubsample = result->h < 8 ? 15 : 5;
#elif STBTT_RASTERIZER_VERSION == 2
   int vsubsample = 1;
#else
   #error "Unrecognized value of STBTT_RASTERIZER_VERSION"
#endif
   // vsubsample should divide 255 evenly; otherwise we won't reach full opacity

   // now we have to blow out the windings into explicit edge lists
   n = 0;
   for (i=0; i < windings; ++i)
      n += wcount[i];

   e = (stbtt__edge *) STBTT_malloc(sizeof(*e) * (n+1), userdata); // add an extra one as a sentinel
   if (e == 0) return;
   n = 0;

   m=0;
   for (i=0; i < windings; ++i) {
      stbtt__point *p = pts + m;
      m += wcount[i];
      j = wcount[i]-1;
      for (k=0; k < wcount[i]; j=k++) {
         int a=k,b=j;
         // skip the edge if horizontal
         if (p[j].y == p[k].y)
            continue;
         // add edge from j to k to the list
         e[n].invert = 0;
         if (invert ? p[j].y > p[k].y : p[j].y < p[k].y) {
            e[n].invert = 1;
            a=j,b=k;
         }
         e[n].x0 = p[a].x * scale_x + shift_x;
         e[n].y0 = (p[a].y * y_scale_inv + shift_y) * vsubsample;
         e[n].x1 = p[b].x * scale_x + shift_x;
         e[n].y1 = (p[b].y * y_scale_inv + shift_y) * vsubsample;
         ++n;
      }
   }

   // now sort the edges by their highest point (should snap to integer, and then by x)
   //STBTT_sort(e, n, sizeof(e[0]), stbtt__edge_compare);
   stbtt__sort_edges(e, n);

   // now, traverse the scanlines and find the intersections on each scanline, use xor winding rule
   stbtt__rasterize_sorted_edges(result, e, n, vsubsample, off_x, off_y, userdata);

   STBTT_free(e, userdata);
}

static void stbtt__add_point(stbtt__point *points, int n, float x, float y)
{
   if (!points) return; // during first pass, it's unallocated
   points[n].x = x;
   points[n].y = y;
}

// tessellate until threshold p is happy... @TODO warped to compensate for non-linear stretching
static int stbtt__tesselate_curve(stbtt__point *points, int *num_points, float x0, float y0, float x1, float y1, float x2, float y2, float objspace_flatness_squared, int n)
{
   // midpoint
   float mx = (x0 + 2*x1 + x2)/4;
   float my = (y0 + 2*y1 + y2)/4;
   // versus directly drawn line
   float dx = (x0+x2)/2 - mx;
   float dy = (y0+y2)/2 - my;
   if (n > 16) // 65536 segments on one curve better be enough!
      return 1;
   if (dx*dx+dy*dy > objspace_flatness_squared) { // half-pixel error allowed... need to be smaller if AA
      stbtt__tesselate_curve(points, num_points, x0,y0, (x0+x1)/2.0f,(y0+y1)/2.0f, mx,my, objspace_flatness_squared,n+1);
      stbtt__tesselate_curve(points, num_points, mx,my, (x1+x2)/2.0f,(y1+y2)/2.0f, x2,y2, objspace_flatness_squared,n+1);
   } else {
      stbtt__add_point(points, *num_points,x2,y2);
      *num_points = *num_points+1;
   }
   return 1;
}

static void stbtt__tesselate_cubic(stbtt__point *points, int *num_points, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float objspace_flatness_squared, int n)
{
   // @TODO this "flatness" calculation is just made-up nonsense that seems to work well enough
   float dx0 = x1-x0;
   float dy0 = y1-y0;
   float dx1 = x2-x1;
   float dy1 = y2-y1;
   float dx2 = x3-x2;
   float dy2 = y3-y2;
   float dx = x3-x0;
   float dy = y3-y0;
   float longlen = (float) (STBTT_sqrt(dx0*dx0+dy0*dy0)+STBTT_sqrt(dx1*dx1+dy1*dy1)+STBTT_sqrt(dx2*dx2+dy2*dy2));
   float shortlen = (float) STBTT_sqrt(dx*dx+dy*dy);
   float flatness_squared = longlen*longlen-shortlen*shortlen;

   if (n > 16) // 65536 segments on one curve better be enough!
      return;

   if (flatness_squared > objspace_flatness_squared) {
      float x01 = (x0+x1)/2;
      float y01 = (y0+y1)/2;
      float x12 = (x1+x2)/2;
      float y12 = (y1+y2)/2;
      float x23 = (x2+x3)/2;
      float y23 = (y2+y3)/2;

      float xa = (x01+x12)/2;
      float ya = (y01+y12)/2;
      float xb = (x12+x23)/2;
      float yb = (y12+y23)/2;

      float mx = (xa+xb)/2;
      float my = (ya+yb)/2;

      stbtt__tesselate_cubic(points, num_points, x0,y0, x01,y01, xa,ya, mx,my, objspace_flatness_squared,n+1);
      stbtt__tesselate_cubic(points, num_points, mx,my, xb,yb, x23,y23, x3,y3, objspace_flatness_squared,n+1);
   } else {
      stbtt__add_point(points, *num_points,x3,y3);
      *num_points = *num_points+1;
   }
}

// returns number of contours
static stbtt__point *stbtt_FlattenCurves(stbtt_vertex *vertices, int num_verts, float objspace_flatness, int **contour_lengths, int *num_contours, void *userdata)
{
   stbtt__point *points=0;
   int num_points=0;

   float objspace_flatness_squared = objspace_flatness * objspace_flatness;
   int i,n=0,start=0, pass;

   // count how many "moves" there are to get the contour count
   for (i=0; i < num_verts; ++i)
      if (vertices[i].type == STBTT_vmove)
         ++n;

   *num_contours = n;
   if (n == 0) return 0;

   *contour_lengths = (int *) STBTT_malloc(sizeof(**contour_lengths) * n, userdata);

   if (*contour_lengths == 0) {
      *num_contours = 0;
      return 0;
   }

   // make two passes through the points so we don't need to realloc
   for (pass=0; pass < 2; ++pass) {
      float x=0,y=0;
      if (pass == 1) {
         points = (stbtt__point *) STBTT_malloc(num_points * sizeof(points[0]), userdata);
         if (points == NULL) goto error;
      }
      num_points = 0;
      n= -1;
      for (i=0; i < num_verts; ++i) {
         switch (vertices[i].type) {
            case STBTT_vmove:
               // start the next contour
               if (n >= 0)
                  (*contour_lengths)[n] = num_points - start;
               ++n;
               start = num_points;

               x = vertices[i].x, y = vertices[i].y;
               stbtt__add_point(points, num_points++, x,y);
               break;
            case STBTT_vline:
               x = vertices[i].x, y = vertices[i].y;
               stbtt__add_point(points, num_points++, x, y);
               break;
            case STBTT_vcurve:
               stbtt__tesselate_curve(points, &num_points, x,y,
                                        vertices[i].cx, vertices[i].cy,
                                        vertices[i].x,  vertices[i].y,
                                        objspace_flatness_squared, 0);
               x = vertices[i].x, y = vertices[i].y;
               break;
            case STBTT_vcubic:
               stbtt__tesselate_cubic(points, &num_points, x,y,
                                        vertices[i].cx, vertices[i].cy,
                                        vertices[i].cx1, vertices[i].cy1,
                                        vertices[i].x,  vertices[i].y,
                                        objspace_flatness_squared, 0);
               x = vertices[i].x, y = vertices[i].y;
               break;
         }
      }
      (*contour_lengths)[n] = num_points - start;
   }

   return points;
error:
   STBTT_free(points, userdata);
   STBTT_free(*contour_lengths, userdata);
   *contour_lengths = 0;
   *num_contours = 0;
   return NULL;
}

STBTT_DEF void stbtt_Rasterize(stbtt__bitmap *result, float flatness_in_pixels, stbtt_vertex *vertices, int num_verts, float scale_x, float scale_y, float shift_x, float shift_y, int x_off, int y_off, int invert, void *userdata)
{
   float scale            = scale_x > scale_y ? scale_y : scale_x;
   int winding_count      = 0;
   int *winding_lengths   = NULL;
   stbtt__point *windings = stbtt_FlattenCurves(vertices, num_verts, flatness_in_pixels / scale, &winding_lengths, &winding_count, userdata);
   if (windings) {
      stbtt__rasterize(result, windings, winding_lengths, winding_count, scale_x, scale_y, shift_x, shift_y, x_off, y_off, invert, userdata);
      STBTT_free(winding_lengths, userdata);
      STBTT_free(windings, userdata);
   }
}

STBTT_DEF void stbtt_FreeBitmap(unsigned char *bitmap, void *userdata)
{
   STBTT_free(bitmap, userdata);
}

STBTT_DEF unsigned char *stbtt_GetGlyphBitmapSubpixel(const stbtt_fontinfo *info, float scale_x, float scale_y, float shift_x, float shift_y, int glyph, int *width, int *height, int *xoff, int *yoff)
{
   int ix0,iy0,ix1,iy1;
   stbtt__bitmap gbm;
   stbtt_vertex *vertices;
   int num_verts = stbtt_GetGlyphShape(info, glyph, &vertices);

   if (scale_x == 0) scale_x = scale_y;
   if (scale_y == 0) {
      if (scale_x == 0) {
         STBTT_free(vertices, info->userdata);
         return NULL;
      }
      scale_y = scale_x;
   }

   stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, &ix0,&iy0,&ix1,&iy1);

   // now we get the size
   gbm.w = (ix1 - ix0);
   gbm.h = (iy1 - iy0);
   gbm.pixels = NULL; // in case we error

   if (width ) *width  = gbm.w;
   if (height) *height = gbm.h;
   if (xoff  ) *xoff   = ix0;
   if (yoff  ) *yoff   = iy0;

   if (gbm.w && gbm.h) {
      gbm.pixels = (unsigned char *) STBTT_malloc(gbm.w * gbm.h, info->userdata);
      if (gbm.pixels) {
         gbm.stride = gbm.w;

         stbtt_Rasterize(&gbm, 0.35f, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0, iy0, 1, info->userdata);
      }
   }
   STBTT_free(vertices, info->userdata);
   return gbm.pixels;
}

STBTT_DEF unsigned char *stbtt_GetGlyphBitmap(const stbtt_fontinfo *info, float scale_x, float scale_y, int glyph, int *width, int *height, int *xoff, int *yoff)
{
   return stbtt_GetGlyphBitmapSubpixel(info, scale_x, scale_y, 0.0f, 0.0f, glyph, width, height, xoff, yoff);
}

STBTT_DEF void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph)
{
   int ix0,iy0;
   stbtt_vertex *vertices;
   int num_verts = stbtt_GetGlyphShape(info, glyph, &vertices);
   stbtt__bitmap gbm;

   stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, &ix0,&iy0,0,0);
   gbm.pixels = output;
   gbm.w = out_w;
   gbm.h = out_h;
   gbm.stride = out_stride;

   if (gbm.w && gbm.h)
      stbtt_Rasterize(&gbm, 0.35f, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0,iy0, 1, info->userdata);

   STBTT_free(vertices, info->userdata);
}

STBTT_DEF void stbtt_MakeGlyphBitmap(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int glyph)
{
   stbtt_MakeGlyphBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, 0.0f,0.0f, glyph);
}

STBTT_DEF unsigned char *stbtt_GetCodepointBitmapSubpixel(const stbtt_fontinfo *info, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint, int *width, int *height, int *xoff, int *yoff)
{
   return stbtt_GetGlyphBitmapSubpixel(info, scale_x, scale_y,shift_x,shift_y, stbtt_FindGlyphIndex(info,codepoint), width,height,xoff,yoff);
}

STBTT_DEF void stbtt_MakeCodepointBitmapSubpixelPrefilter(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int oversample_x, int oversample_y, float *sub_x, float *sub_y, int codepoint)
{
   stbtt_MakeGlyphBitmapSubpixelPrefilter(info, output, out_w, out_h, out_stride, scale_x, scale_y, shift_x, shift_y, oversample_x, oversample_y, sub_x, sub_y, stbtt_FindGlyphIndex(info,codepoint));
}

STBTT_DEF void stbtt_MakeCodepointBitmapSubpixel(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint)
{
   stbtt_MakeGlyphBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, shift_x, shift_y, stbtt_FindGlyphIndex(info,codepoint));
}

STBTT_DEF unsigned char *stbtt_GetCodepointBitmap(const stbtt_fontinfo *info, float scale_x, float scale_y, int codepoint, int *width, int *height, int *xoff, int *yoff)
{
   return stbtt_GetCodepointBitmapSubpixel(info, scale_x, scale_y, 0.0f,0.0f, codepoint, width,height,xoff,yoff);
}

STBTT_DEF void stbtt_MakeCodepointBitmap(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int codepoint)
{
   stbtt_MakeCodepointBitmapSubpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, 0.0f,0.0f, codepoint);
}

//////////////////////////////////////////////////////////////////////////////
//
// bitmap baking
//
// This is SUPER-CRAPPY packing to keep source code small

static int stbtt_BakeFontBitmap_internal(unsigned char *data, int offset,  // font location (use offset=0 for plain .ttf)
                                float pixel_height,                     // height of font in pixels
                                unsigned char *pixels, int pw, int ph,  // bitmap to be filled in
                                int first_char, int num_chars,          // characters to bake
                                stbtt_bakedchar *chardata)
{
   float scale;
   int x,y,bottom_y, i;
   stbtt_fontinfo f;
   f.userdata = NULL;
   if (!stbtt_InitFont(&f, data, offset))
      return -1;
   STBTT_memset(pixels, 0, pw*ph); // background of 0 around pixels
   x=y=1;
   bottom_y = 1;

   scale = stbtt_ScaleForPixelHeight(&f, pixel_height);

   for (i=0; i < num_chars; ++i) {
      int advance, lsb, x0,y0,x1,y1,gw,gh;
      int g = stbtt_FindGlyphIndex(&f, first_char + i);
      stbtt_GetGlyphHMetrics(&f, g, &advance, &lsb);
      stbtt_GetGlyphBitmapBox(&f, g, scale,scale, &x0,&y0,&x1,&y1);
      gw = x1-x0;
      gh = y1-y0;
      if (x + gw + 1 >= pw)
         y = bottom_y, x = 1; // advance to next row
      if (y + gh + 1 >= ph) // check if it fits vertically AFTER potentially moving to next row
         return -i;
      STBTT_assert(x+gw < pw);
      STBTT_assert(y+gh < ph);
      stbtt_MakeGlyphBitmap(&f, pixels+x+y*pw, gw,gh,pw, scale,scale, g);
      chardata[i].x0 = (stbtt_int16) x;
      chardata[i].y0 = (stbtt_int16) y;
      chardata[i].x1 = (stbtt_int16) (x + gw);
      chardata[i].y1 = (stbtt_int16) (y + gh);
      chardata[i].xadvance = scale * advance;
      chardata[i].xoff     = (float) x0;
      chardata[i].yoff     = (float) y0;
      x = x + gw + 1;
      if (y+gh+1 > bottom_y)
         bottom_y = y+gh+1;
   }
   return bottom_y;
}

STBTT_DEF void stbtt_GetBakedQuad(const stbtt_bakedchar *chardata, int pw, int ph, int char_index, float *xpos, float *ypos, stbtt_aligned_quad *q, int opengl_fillrule)
{
   float d3d_bias = opengl_fillrule ? 0 : -0.5f;
   float ipw = 1.0f / pw, iph = 1.0f / ph;
   const stbtt_bakedchar *b = chardata + char_index;
   int round_x = STBTT_ifloor((*xpos + b->xoff) + 0.5f);
   int round_y = STBTT_ifloor((*ypos + b->yoff) + 0.5f);

   q->x0 = round_x + d3d_bias;
   q->y0 = round_y + d3d_bias;
   q->x1 = round_x + b->x1 - b->x0 + d3d_bias;
   q->y1 = round_y + b->y1 - b->y0 + d3d_bias;

   q->s0 = b->x0 * ipw;
   q->t0 = b->y0 * iph;
   q->s1 = b->x1 * ipw;
   q->t1 = b->y1 * iph;

   *xpos += b->xadvance;
}

//////////////////////////////////////////////////////////////////////////////
//
// rectangle packing replacement routines if you don't have stb_rect_pack.h
//

#ifndef STB_RECT_PACK_VERSION

typedef int stbrp_coord;

////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
//                                                                                //
// COMPILER WARNING ?!?!?                                                         //
//                                                                                //
//                                                                                //
// if you get a compile warning due to these symbols being defined more than      //
// once, move #include "stb_rect_pack.h" before #include "stb_truetype.h"         //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
   int width,height;
   int x,y,bottom_y;
} stbrp_context;

typedef struct
{
   unsigned char x;
} stbrp_node;

struct stbrp_rect
{
   stbrp_coord x,y;
   int id,w,h,was_packed;
};

static void stbrp_init_target(stbrp_context *con, int pw, int ph, stbrp_node *nodes, int num_nodes)
{
   con->width  = pw;
   con->height = ph;
   con->x = 0;
   con->y = 0;
   con->bottom_y = 0;
   STBTT__NOTUSED(nodes);
   STBTT__NOTUSED(num_nodes);
}

static void stbrp_pack_rects(stbrp_context *con, stbrp_rect *rects, int num_rects)
{
   int i;
   for (i=0; i < num_rects; ++i) {
      if (con->x + rects[i].w > con->width) {
         con->x = 0;
         con->y = con->bottom_y;
      }
      if (con->y + rects[i].h > con->height)
         break;
      rects[i].x = con->x;
      rects[i].y = con->y;
      rects[i].was_packed = 1;
      con->x += rects[i].w;
      if (con->y + rects[i].h > con->bottom_y)
         con->bottom_y = con->y + rects[i].h;
   }
   for (   ; i < num_rects; ++i)
      rects[i].was_packed = 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// bitmap baking
//
// This is SUPER-AWESOME (tm Ryan Gordon) packing using stb_rect_pack.h. If
// stb_rect_pack.h isn't available, it uses the BakeFontBitmap strategy.

STBTT_DEF int stbtt_PackBegin(stbtt_pack_context *spc, unsigned char *pixels, int pw, int ph, int stride_in_bytes, int padding, void *alloc_context)
{
   stbrp_context *context = (stbrp_context *) STBTT_malloc(sizeof(*context)            ,alloc_context);
   int            num_nodes = pw - padding;
   stbrp_node    *nodes   = (stbrp_node    *) STBTT_malloc(sizeof(*nodes  ) * num_nodes,alloc_context);

   if (context == NULL || nodes == NULL) {
      if (context != NULL) STBTT_free(context, alloc_context);
      if (nodes   != NULL) STBTT_free(nodes  , alloc_context);
      return 0;
   }

   spc->user_allocator_context = alloc_context;
   spc->width = pw;
   spc->height = ph;
   spc->pixels = pixels;
   spc->pack_info = context;
   spc->nodes = nodes;
   spc->padding = padding;
   spc->stride_in_bytes = stride_in_bytes != 0 ? stride_in_bytes : pw;
   spc->h_oversample = 1;
   spc->v_oversample = 1;
   spc->skip_missing = 0;

   stbrp_init_target(context, pw-padding, ph-padding, nodes, num_nodes);

   if (pixels)
      STBTT_memset(pixels, 0, pw*ph); // background of 0 around pixels

   return 1;
}

STBTT_DEF void stbtt_PackEnd  (stbtt_pack_context *spc)
{
   STBTT_free(spc->nodes    , spc->user_allocator_context);
   STBTT_free(spc->pack_info, spc->user_allocator_context);
}

STBTT_DEF void stbtt_PackSetOversampling(stbtt_pack_context *spc, unsigned int h_oversample, unsigned int v_oversample)
{
   STBTT_assert(h_oversample <= STBTT_MAX_OVERSAMPLE);
   STBTT_assert(v_oversample <= STBTT_MAX_OVERSAMPLE);
   if (h_oversample <= STBTT_MAX_OVERSAMPLE)
      spc->h_oversample = h_oversample;
   if (v_oversample <= STBTT_MAX_OVERSAMPLE)
      spc->v_oversample = v_oversample;
}

STBTT_DEF void stbtt_PackSetSkipMissingCodepoints(stbtt_pack_context *spc, int skip)
{
   spc->skip_missing = skip;
}

#define STBTT__OVER_MASK  (STBTT_MAX_OVERSAMPLE-1)

static void stbtt__h_prefilter(unsigned char *pixels, int w, int h, int stride_in_bytes, unsigned int kernel_width)
{
   unsigned char buffer[STBTT_MAX_OVERSAMPLE];
   int safe_w = w - kernel_width;
   int j;
   STBTT_memset(buffer, 0, STBTT_MAX_OVERSAMPLE); // suppress bogus warning from VS2013 -analyze
   for (j=0; j < h; ++j) {
      int i;
      unsigned int total;
      STBTT_memset(buffer, 0, kernel_width);

      total = 0;

      // make kernel_width a constant in common cases so compiler can optimize out the divide
      switch (kernel_width) {
         case 2:
            for (i=0; i <= safe_w; ++i) {
               total += pixels[i] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / 2);
            }
            break;
         case 3:
            for (i=0; i <= safe_w; ++i) {
               total += pixels[i] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / 3);
            }
            break;
         case 4:
            for (i=0; i <= safe_w; ++i) {
               total += pixels[i] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / 4);
            }
            break;
         case 5:
            for (i=0; i <= safe_w; ++i) {
               total += pixels[i] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / 5);
            }
            break;
         default:
            for (i=0; i <= safe_w; ++i) {
               total += pixels[i] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i];
               pixels[i] = (unsigned char) (total / kernel_width);
            }
            break;
      }

      for (; i < w; ++i) {
         STBTT_assert(pixels[i] == 0);
         total -= buffer[i & STBTT__OVER_MASK];
         pixels[i] = (unsigned char) (total / kernel_width);
      }

      pixels += stride_in_bytes;
   }
}

static void stbtt__v_prefilter(unsigned char *pixels, int w, int h, int stride_in_bytes, unsigned int kernel_width)
{
   unsigned char buffer[STBTT_MAX_OVERSAMPLE];
   int safe_h = h - kernel_width;
   int j;
   STBTT_memset(buffer, 0, STBTT_MAX_OVERSAMPLE); // suppress bogus warning from VS2013 -analyze
   for (j=0; j < w; ++j) {
      int i;
      unsigned int total;
      STBTT_memset(buffer, 0, kernel_width);

      total = 0;

      // make kernel_width a constant in common cases so compiler can optimize out the divide
      switch (kernel_width) {
         case 2:
            for (i=0; i <= safe_h; ++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / 2);
            }
            break;
         case 3:
            for (i=0; i <= safe_h; ++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / 3);
            }
            break;
         case 4:
            for (i=0; i <= safe_h; ++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / 4);
            }
            break;
         case 5:
            for (i=0; i <= safe_h; ++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / 5);
            }
            break;
         default:
            for (i=0; i <= safe_h; ++i) {
               total += pixels[i*stride_in_bytes] - buffer[i & STBTT__OVER_MASK];
               buffer[(i+kernel_width) & STBTT__OVER_MASK] = pixels[i*stride_in_bytes];
               pixels[i*stride_in_bytes] = (unsigned char) (total / kernel_width);
            }
            break;
      }

      for (; i < h; ++i) {
         STBTT_assert(pixels[i*stride_in_bytes] == 0);
         total -= buffer[i & STBTT__OVER_MASK];
         pixels[i*stride_in_bytes] = (unsigned char) (total / kernel_width);
      }

      pixels += 1;
   }
}

static float stbtt__oversample_shift(int oversample)
{
   if (!oversample)
      return 0.0f;

   // The prefilter is a box filter of width "oversample",
   // which shifts phase by (oversample - 1)/2 pixels in
   // oversampled space. We want to shift in the opposite
   // direction to counter this.
   return (float)-(oversample - 1) / (2.0f * (float)oversample);
}

// rects array must be big enough to accommodate all characters in the given ranges
STBTT_DEF int stbtt_PackFontRangesGatherRects(stbtt_pack_context *spc, const stbtt_fontinfo *info, stbtt_pack_range *ranges, int num_ranges, stbrp_rect *rects)
{
   int i,j,k;
   int missing_glyph_added = 0;

   k=0;
   for (i=0; i < num_ranges; ++i) {
      float fh = ranges[i].font_size;
      float scale = fh > 0 ? stbtt_ScaleForPixelHeight(info, fh) : stbtt_ScaleForMappingEmToPixels(info, -fh);
      ranges[i].h_oversample = (unsigned char) spc->h_oversample;
      ranges[i].v_oversample = (unsigned char) spc->v_oversample;
      for (j=0; j < ranges[i].num_chars; ++j) {
         int x0,y0,x1,y1;
         int codepoint = ranges[i].array_of_unicode_codepoints == NULL ? ranges[i].first_unicode_codepoint_in_range + j : ranges[i].array_of_unicode_codepoints[j];
         int glyph = stbtt_FindGlyphIndex(info, codepoint);
         if (glyph == 0 && (spc->skip_missing || missing_glyph_added)) {
            rects[k].w = rects[k].h = 0;
         } else {
            stbtt_GetGlyphBitmapBoxSubpixel(info,glyph,
                                            scale * spc->h_oversample,
                                            scale * spc->v_oversample,
                                            0,0,
                                            &x0,&y0,&x1,&y1);
            rects[k].w = (stbrp_coord) (x1-x0 + spc->padding + spc->h_oversample-1);
            rects[k].h = (stbrp_coord) (y1-y0 + spc->padding + spc->v_oversample-1);
            if (glyph == 0)
               missing_glyph_added = 1;
         }
         ++k;
      }
   }

   return k;
}

STBTT_DEF void stbtt_MakeGlyphBitmapSubpixelPrefilter(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int prefilter_x, int prefilter_y, float *sub_x, float *sub_y, int glyph)
{
   stbtt_MakeGlyphBitmapSubpixel(info,
                                 output,
                                 out_w - (prefilter_x - 1),
                                 out_h - (prefilter_y - 1),
                                 out_stride,
                                 scale_x,
                                 scale_y,
                                 shift_x,
                                 shift_y,
                                 glyph);

   if (prefilter_x > 1)
      stbtt__h_prefilter(output, out_w, out_h, out_stride, prefilter_x);

   if (prefilter_y > 1)
      stbtt__v_prefilter(output, out_w, out_h, out_stride, prefilter_y);

   *sub_x = stbtt__oversample_shift(prefilter_x);
   *sub_y = stbtt__oversample_shift(prefilter_y);
}

// rects array must be big enough to accommodate all characters in the given ranges
STBTT_DEF int stbtt_PackFontRangesRenderIntoRects(stbtt_pack_context *spc, const stbtt_fontinfo *info, stbtt_pack_range *ranges, int num_ranges, stbrp_rect *rects)
{
   int i,j,k, missing_glyph = -1, return_value = 1;

   // save current values
   int old_h_over = spc->h_oversample;
   int old_v_over = spc->v_oversample;

   k = 0;
   for (i=0; i < num_ranges; ++i) {
      float fh = ranges[i].font_size;
      float scale = fh > 0 ? stbtt_ScaleForPixelHeight(info, fh) : stbtt_ScaleForMappingEmToPixels(info, -fh);
      float recip_h,recip_v,sub_x,sub_y;
      spc->h_oversample = ranges[i].h_oversample;
      spc->v_oversample = ranges[i].v_oversample;
      recip_h = 1.0f / spc->h_oversample;
      recip_v = 1.0f / spc->v_oversample;
      sub_x = stbtt__oversample_shift(spc->h_oversample);
      sub_y = stbtt__oversample_shift(spc->v_oversample);
      for (j=0; j < ranges[i].num_chars; ++j) {
         stbrp_rect *r = &rects[k];
         if (r->was_packed && r->w != 0 && r->h != 0) {
            stbtt_packedchar *bc = &ranges[i].chardata_for_range[j];
            int advance, lsb, x0,y0,x1,y1;
            int codepoint = ranges[i].array_of_unicode_codepoints == NULL ? ranges[i].first_unicode_codepoint_in_range + j : ranges[i].array_of_unicode_codepoints[j];
            int glyph = stbtt_FindGlyphIndex(info, codepoint);
            stbrp_coord pad = (stbrp_coord) spc->padding;

            // pad on left and top
            r->x += pad;
            r->y += pad;
            r->w -= pad;
            r->h -= pad;
            stbtt_GetGlyphHMetrics(info, glyph, &advance, &lsb);
            stbtt_GetGlyphBitmapBox(info, glyph,
                                    scale * spc->h_oversample,
                                    scale * spc->v_oversample,
                                    &x0,&y0,&x1,&y1);
            stbtt_MakeGlyphBitmapSubpixel(info,
                                          spc->pixels + r->x + r->y*spc->stride_in_bytes,
                                          r->w - spc->h_oversample+1,
                                          r->h - spc->v_oversample+1,
                                          spc->stride_in_bytes,
                                          scale * spc->h_oversample,
                                          scale * spc->v_oversample,
                                          0,0,
                                          glyph);

            if (spc->h_oversample > 1)
               stbtt__h_prefilter(spc->pixels + r->x + r->y*spc->stride_in_bytes,
                                  r->w, r->h, spc->stride_in_bytes,
                                  spc->h_oversample);

            if (spc->v_oversample > 1)
               stbtt__v_prefilter(spc->pixels + r->x + r->y*spc->stride_in_bytes,
                                  r->w, r->h, spc->stride_in_bytes,
                                  spc->v_oversample);

            bc->x0       = (stbtt_int16)  r->x;
            bc->y0       = (stbtt_int16)  r->y;
            bc->x1       = (stbtt_int16) (r->x + r->w);
            bc->y1       = (stbtt_int16) (r->y + r->h);
            bc->xadvance =                scale * advance;
            bc->xoff     =       (float)  x0 * recip_h + sub_x;
            bc->yoff     =       (float)  y0 * recip_v + sub_y;
            bc->xoff2    =                (x0 + r->w) * recip_h + sub_x;
            bc->yoff2    =                (y0 + r->h) * recip_v + sub_y;

            if (glyph == 0)
               missing_glyph = j;
         } else if (spc->skip_missing) {
            return_value = 0;
         } else if (r->was_packed && r->w == 0 && r->h == 0 && missing_glyph >= 0) {
            ranges[i].chardata_for_range[j] = ranges[i].chardata_for_range[missing_glyph];
         } else {
            return_value = 0; // if any fail, report failure
         }

         ++k;
      }
   }

   // restore original values
   spc->h_oversample = old_h_over;
   spc->v_oversample = old_v_over;

   return return_value;
}

STBTT_DEF void stbtt_PackFontRangesPackRects(stbtt_pack_context *spc, stbrp_rect *rects, int num_rects)
{
   stbrp_pack_rects((stbrp_context *) spc->pack_info, rects, num_rects);
}

STBTT_DEF int stbtt_PackFontRanges(stbtt_pack_context *spc, const unsigned char *fontdata, int font_index, stbtt_pack_range *ranges, int num_ranges)
{
   stbtt_fontinfo info;
   int i,j,n, return_value = 1;
   //stbrp_context *context = (stbrp_context *) spc->pack_info;
   stbrp_rect    *rects;

   // flag all characters as NOT packed
   for (i=0; i < num_ranges; ++i)
      for (j=0; j < ranges[i].num_chars; ++j)
         ranges[i].chardata_for_range[j].x0 =
         ranges[i].chardata_for_range[j].y0 =
         ranges[i].chardata_for_range[j].x1 =
         ranges[i].chardata_for_range[j].y1 = 0;

   n = 0;
   for (i=0; i < num_ranges; ++i)
      n += ranges[i].num_chars;

   rects = (stbrp_rect *) STBTT_malloc(sizeof(*rects) * n, spc->user_allocator_context);
   if (rects == NULL)
      return 0;

   info.userdata = spc->user_allocator_context;
   stbtt_InitFont(&info, fontdata, stbtt_GetFontOffsetForIndex(fontdata,font_index));

   n = stbtt_PackFontRangesGatherRects(spc, &info, ranges, num_ranges, rects);

   stbtt_PackFontRangesPackRects(spc, rects, n);

   return_value = stbtt_PackFontRangesRenderIntoRects(spc, &info, ranges, num_ranges, rects);

   STBTT_free(rects, spc->user_allocator_context);
   return return_value;
}

STBTT_DEF int stbtt_PackFontRange(stbtt_pack_context *spc, const unsigned char *fontdata, int font_index, float font_size,
            int first_unicode_codepoint_in_range, int num_chars_in_range, stbtt_packedchar *chardata_for_range)
{
   stbtt_pack_range range;
   range.first_unicode_codepoint_in_range = first_unicode_codepoint_in_range;
   range.array_of_unicode_codepoints = NULL;
   range.num_chars                   = num_chars_in_range;
   range.chardata_for_range          = chardata_for_range;
   range.font_size                   = font_size;
   return stbtt_PackFontRanges(spc, fontdata, font_index, &range, 1);
}

STBTT_DEF void stbtt_GetScaledFontVMetrics(const unsigned char *fontdata, int index, float size, float *ascent, float *descent, float *lineGap)
{
   int i_ascent, i_descent, i_lineGap;
   float scale;
   stbtt_fontinfo info;
   stbtt_InitFont(&info, fontdata, stbtt_GetFontOffsetForIndex(fontdata, index));
   scale = size > 0 ? stbtt_ScaleForPixelHeight(&info, size) : stbtt_ScaleForMappingEmToPixels(&info, -size);
   stbtt_GetFontVMetrics(&info, &i_ascent, &i_descent, &i_lineGap);
   *ascent  = (float) i_ascent  * scale;
   *descent = (float) i_descent * scale;
   *lineGap = (float) i_lineGap * scale;
}

STBTT_DEF void stbtt_GetPackedQuad(const stbtt_packedchar *chardata, int pw, int ph, int char_index, float *xpos, float *ypos, stbtt_aligned_quad *q, int align_to_integer)
{
   float ipw = 1.0f / pw, iph = 1.0f / ph;
   const stbtt_packedchar *b = chardata + char_index;

   if (align_to_integer) {
      float x = (float) STBTT_ifloor((*xpos + b->xoff) + 0.5f);
      float y = (float) STBTT_ifloor((*ypos + b->yoff) + 0.5f);
      q->x0 = x;
      q->y0 = y;
      q->x1 = x + b->xoff2 - b->xoff;
      q->y1 = y + b->yoff2 - b->yoff;
   } else {
      q->x0 = *xpos + b->xoff;
      q->y0 = *ypos + b->yoff;
      q->x1 = *xpos + b->xoff2;
      q->y1 = *ypos + b->yoff2;
   }

   q->s0 = b->x0 * ipw;
   q->t0 = b->y0 * iph;
   q->s1 = b->x1 * ipw;
   q->t1 = b->y1 * iph;

   *xpos += b->xadvance;
}

//////////////////////////////////////////////////////////////////////////////
//
// sdf computation
//

#define STBTT_min(a,b)  ((a) < (b) ? (a) : (b))
#define STBTT_max(a,b)  ((a) < (b) ? (b) : (a))

static int stbtt__ray_intersect_bezier(float orig[2], float ray[2], float q0[2], float q1[2], float q2[2], float hits[2][2])
{
   float q0perp = q0[1]*ray[0] - q0[0]*ray[1];
   float q1perp = q1[1]*ray[0] - q1[0]*ray[1];
   float q2perp = q2[1]*ray[0] - q2[0]*ray[1];
   float roperp = orig[1]*ray[0] - orig[0]*ray[1];

   float a = q0perp - 2*q1perp + q2perp;
   float b = q1perp - q0perp;
   float c = q0perp - roperp;

   float s0 = 0., s1 = 0.;
   int num_s = 0;

   if (a != 0.0) {
      float discr = b*b - a*c;
      if (discr > 0.0) {
         float rcpna = -1 / a;
         float d = (float) STBTT_sqrt(discr);
         s0 = (b+d) * rcpna;
         s1 = (b-d) * rcpna;
         if (s0 >= 0.0 && s0 <= 1.0)
            num_s = 1;
         if (d > 0.0 && s1 >= 0.0 && s1 <= 1.0) {
            if (num_s == 0) s0 = s1;
            ++num_s;
         }
      }
   } else {
      // 2*b*s + c = 0
      // s = -c / (2*b)
      s0 = c / (-2 * b);
      if (s0 >= 0.0 && s0 <= 1.0)
         num_s = 1;
   }

   if (num_s == 0)
      return 0;
   else {
      float rcp_len2 = 1 / (ray[0]*ray[0] + ray[1]*ray[1]);
      float rayn_x = ray[0] * rcp_len2, rayn_y = ray[1] * rcp_len2;

      float q0d =   q0[0]*rayn_x +   q0[1]*rayn_y;
      float q1d =   q1[0]*rayn_x +   q1[1]*rayn_y;
      float q2d =   q2[0]*rayn_x +   q2[1]*rayn_y;
      float rod = orig[0]*rayn_x + orig[1]*rayn_y;

      float q10d = q1d - q0d;
      float q20d = q2d - q0d;
      float q0rd = q0d - rod;

      hits[0][0] = q0rd + s0*(2.0f - 2.0f*s0)*q10d + s0*s0*q20d;
      hits[0][1] = a*s0+b;

      if (num_s > 1) {
         hits[1][0] = q0rd + s1*(2.0f - 2.0f*s1)*q10d + s1*s1*q20d;
         hits[1][1] = a*s1+b;
         return 2;
      } else {
         return 1;
      }
   }
}

static int equal(float *a, float *b)
{
   return (a[0] == b[0] && a[1] == b[1]);
}

static int stbtt__compute_crossings_x(float x, float y, int nverts, stbtt_vertex *verts)
{
   int i;
   float orig[2], ray[2] = { 1, 0 };
   float y_frac;
   int winding = 0;

   // make sure y never passes through a vertex of the shape
   y_frac = (float) STBTT_fmod(y, 1.0f);
   if (y_frac < 0.01f)
      y += 0.01f;
   else if (y_frac > 0.99f)
      y -= 0.01f;

   orig[0] = x;
   orig[1] = y;

   // test a ray from (-infinity,y) to (x,y)
   for (i=0; i < nverts; ++i) {
      if (verts[i].type == STBTT_vline) {
         int x0 = (int) verts[i-1].x, y0 = (int) verts[i-1].y;
         int x1 = (int) verts[i  ].x, y1 = (int) verts[i  ].y;
         if (y > STBTT_min(y0,y1) && y < STBTT_max(y0,y1) && x > STBTT_min(x0,x1)) {
            float x_inter = (y - y0) / (y1 - y0) * (x1-x0) + x0;
            if (x_inter < x)
               winding += (y0 < y1) ? 1 : -1;
         }
      }
      if (verts[i].type == STBTT_vcurve) {
         int x0 = (int) verts[i-1].x , y0 = (int) verts[i-1].y ;
         int x1 = (int) verts[i  ].cx, y1 = (int) verts[i  ].cy;
         int x2 = (int) verts[i  ].x , y2 = (int) verts[i  ].y ;
         int ax = STBTT_min(x0,STBTT_min(x1,x2)), ay = STBTT_min(y0,STBTT_min(y1,y2));
         int by = STBTT_max(y0,STBTT_max(y1,y2));
         if (y > ay && y < by && x > ax) {
            float q0[2],q1[2],q2[2];
            float hits[2][2];
            q0[0] = (float)x0;
            q0[1] = (float)y0;
            q1[0] = (float)x1;
            q1[1] = (float)y1;
            q2[0] = (float)x2;
            q2[1] = (float)y2;
            if (equal(q0,q1) || equal(q1,q2)) {
               x0 = (int)verts[i-1].x;
               y0 = (int)verts[i-1].y;
               x1 = (int)verts[i  ].x;
               y1 = (int)verts[i  ].y;
               if (y > STBTT_min(y0,y1) && y < STBTT_max(y0,y1) && x > STBTT_min(x0,x1)) {
                  float x_inter = (y - y0) / (y1 - y0) * (x1-x0) + x0;
                  if (x_inter < x)
                     winding += (y0 < y1) ? 1 : -1;
               }
            } else {
               int num_hits = stbtt__ray_intersect_bezier(orig, ray, q0, q1, q2, hits);
               if (num_hits >= 1)
                  if (hits[0][0] < 0)
                     winding += (hits[0][1] < 0 ? -1 : 1);
               if (num_hits >= 2)
                  if (hits[1][0] < 0)
                     winding += (hits[1][1] < 0 ? -1 : 1);
            }
         }
      }
   }
   return winding;
}

static float stbtt__cuberoot( float x )
{
   if (x<0)
      return -(float) STBTT_pow(-x,1.0f/3.0f);
   else
      return  (float) STBTT_pow( x,1.0f/3.0f);
}

// x^3 + a*x^2 + b*x + c = 0
static int stbtt__solve_cubic(float a, float b, float c, float* r)
{
   float s = -a / 3;
   float p = b - a*a / 3;
   float q = a * (2*a*a - 9*b) / 27 + c;
   float p3 = p*p*p;
   float d = q*q + 4*p3 / 27;
   if (d >= 0) {
      float z = (float) STBTT_sqrt(d);
      float u = (-q + z) / 2;
      float v = (-q - z) / 2;
      u = stbtt__cuberoot(u);
      v = stbtt__cuberoot(v);
      r[0] = s + u + v;
      return 1;
   } else {
      float u = (float) STBTT_sqrt(-p/3);
      float v = (float) STBTT_acos(-STBTT_sqrt(-27/p3) * q / 2) / 3; // p3 must be negative, since d is negative
      float m = (float) STBTT_cos(v);
      float n = (float) STBTT_cos(v-3.141592/2)*1.732050808f;
      r[0] = s + u * 2 * m;
      r[1] = s - u * (m + n);
      r[2] = s - u * (m - n);

      //STBTT_assert( STBTT_fabs(((r[0]+a)*r[0]+b)*r[0]+c) < 0.05f);  // these asserts may not be safe at all scales, though they're in bezier t parameter units so maybe?
      //STBTT_assert( STBTT_fabs(((r[1]+a)*r[1]+b)*r[1]+c) < 0.05f);
      //STBTT_assert( STBTT_fabs(((r[2]+a)*r[2]+b)*r[2]+c) < 0.05f);
      return 3;
   }
}

STBTT_DEF unsigned char * stbtt_GetGlyphSDF(const stbtt_fontinfo *info, float scale, int glyph, int padding, unsigned char onedge_value, float pixel_dist_scale, int *width, int *height, int *xoff, int *yoff)
{
   float scale_x = scale, scale_y = scale;
   int ix0,iy0,ix1,iy1;
   int w,h;
   unsigned char *data;

   if (scale == 0) return NULL;

   stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale, scale, 0.0f,0.0f, &ix0,&iy0,&ix1,&iy1);

   // if empty, return NULL
   if (ix0 == ix1 || iy0 == iy1)
      return NULL;

   ix0 -= padding;
   iy0 -= padding;
   ix1 += padding;
   iy1 += padding;

   w = (ix1 - ix0);
   h = (iy1 - iy0);

   if (width ) *width  = w;
   if (height) *height = h;
   if (xoff  ) *xoff   = ix0;
   if (yoff  ) *yoff   = iy0;

   // invert for y-downwards bitmaps
   scale_y = -scale_y;

   {
      // distance from singular values (in the same units as the pixel grid)
      const float eps = 1./1024, eps2 = eps*eps;
      int x,y,i,j;
      float *precompute;
      stbtt_vertex *verts;
      int num_verts = stbtt_GetGlyphShape(info, glyph, &verts);
      data = (unsigned char *) STBTT_malloc(w * h, info->userdata);
      precompute = (float *) STBTT_malloc(num_verts * sizeof(float), info->userdata);

      for (i=0,j=num_verts-1; i < num_verts; j=i++) {
         if (verts[i].type == STBTT_vline) {
            float x0 = verts[i].x*scale_x, y0 = verts[i].y*scale_y;
            float x1 = verts[j].x*scale_x, y1 = verts[j].y*scale_y;
            float dist = (float) STBTT_sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
            precompute[i] = (dist < eps) ? 0.0f : 1.0f / dist;
         } else if (verts[i].type == STBTT_vcurve) {
            float x2 = verts[j].x *scale_x, y2 = verts[j].y *scale_y;
            float x1 = verts[i].cx*scale_x, y1 = verts[i].cy*scale_y;
            float x0 = verts[i].x *scale_x, y0 = verts[i].y *scale_y;
            float bx = x0 - 2*x1 + x2, by = y0 - 2*y1 + y2;
            float len2 = bx*bx + by*by;
            if (len2 >= eps2)
               precompute[i] = 1.0f / len2;
            else
               precompute[i] = 0.0f;
         } else
            precompute[i] = 0.0f;
      }

      for (y=iy0; y < iy1; ++y) {
         for (x=ix0; x < ix1; ++x) {
            float val;
            float min_dist = 999999.0f;
            float sx = (float) x + 0.5f;
            float sy = (float) y + 0.5f;
            float x_gspace = (sx / scale_x);
            float y_gspace = (sy / scale_y);

            int winding = stbtt__compute_crossings_x(x_gspace, y_gspace, num_verts, verts); // @OPTIMIZE: this could just be a rasterization, but needs to be line vs. non-tesselated curves so a new path

            for (i=0; i < num_verts; ++i) {
               float x0 = verts[i].x*scale_x, y0 = verts[i].y*scale_y;

               if (verts[i].type == STBTT_vline && precompute[i] != 0.0f) {
                  float x1 = verts[i-1].x*scale_x, y1 = verts[i-1].y*scale_y;

                  float dist,dist2 = (x0-sx)*(x0-sx) + (y0-sy)*(y0-sy);
                  if (dist2 < min_dist*min_dist)
                     min_dist = (float) STBTT_sqrt(dist2);

                  // coarse culling against bbox
                  //if (sx > STBTT_min(x0,x1)-min_dist && sx < STBTT_max(x0,x1)+min_dist &&
                  //    sy > STBTT_min(y0,y1)-min_dist && sy < STBTT_max(y0,y1)+min_dist)
                  dist = (float) STBTT_fabs((x1-x0)*(y0-sy) - (y1-y0)*(x0-sx)) * precompute[i];
                  STBTT_assert(i != 0);
                  if (dist < min_dist) {
                     // check position along line
                     // x' = x0 + t*(x1-x0), y' = y0 + t*(y1-y0)
                     // minimize (x'-sx)*(x'-sx)+(y'-sy)*(y'-sy)
                     float dx = x1-x0, dy = y1-y0;
                     float px = x0-sx, py = y0-sy;
                     // minimize (px+t*dx)^2 + (py+t*dy)^2 = px*px + 2*px*dx*t + t^2*dx*dx + py*py + 2*py*dy*t + t^2*dy*dy
                     // derivative: 2*px*dx + 2*py*dy + (2*dx*dx+2*dy*dy)*t, set to 0 and solve
                     float t = -(px*dx + py*dy) / (dx*dx + dy*dy);
                     if (t >= 0.0f && t <= 1.0f)
                        min_dist = dist;
                  }
               } else if (verts[i].type == STBTT_vcurve) {
                  float x2 = verts[i-1].x *scale_x, y2 = verts[i-1].y *scale_y;
                  float x1 = verts[i  ].cx*scale_x, y1 = verts[i  ].cy*scale_y;
                  float box_x0 = STBTT_min(STBTT_min(x0,x1),x2);
                  float box_y0 = STBTT_min(STBTT_min(y0,y1),y2);
                  float box_x1 = STBTT_max(STBTT_max(x0,x1),x2);
                  float box_y1 = STBTT_max(STBTT_max(y0,y1),y2);
                  // coarse culling against bbox to avoid computing cubic unnecessarily
                  if (sx > box_x0-min_dist && sx < box_x1+min_dist && sy > box_y0-min_dist && sy < box_y1+min_dist) {
                     int num=0;
                     float ax = x1-x0, ay = y1-y0;
                     float bx = x0 - 2*x1 + x2, by = y0 - 2*y1 + y2;
                     float mx = x0 - sx, my = y0 - sy;
                     float res[3] = {0.f,0.f,0.f};
                     float px,py,t,it,dist2;
                     float a_inv = precompute[i];
                     if (a_inv == 0.0) { // if a_inv is 0, it's 2nd degree so use quadratic formula
                        float a = 3*(ax*bx + ay*by);
                        float b = 2*(ax*ax + ay*ay) + (mx*bx+my*by);
                        float c = mx*ax+my*ay;
                        if (STBTT_fabs(a) < eps2) { // if a is 0, it's linear
                           if (STBTT_fabs(b) >= eps2) {
                              res[num++] = -c/b;
                           }
                        } else {
                           float discriminant = b*b - 4*a*c;
                           if (discriminant < 0)
                              num = 0;
                           else {
                              float root = (float) STBTT_sqrt(discriminant);
                              res[0] = (-b - root)/(2*a);
                              res[1] = (-b + root)/(2*a);
                              num = 2; // don't bother distinguishing 1-solution case, as code below will still work
                           }
                        }
                     } else {
                        float b = 3*(ax*bx + ay*by) * a_inv; // could precompute this as it doesn't depend on sample point
                        float c = (2*(ax*ax + ay*ay) + (mx*bx+my*by)) * a_inv;
                        float d = (mx*ax+my*ay) * a_inv;
                        num = stbtt__solve_cubic(b, c, d, res);
                     }
                     dist2 = (x0-sx)*(x0-sx) + (y0-sy)*(y0-sy);
                     if (dist2 < min_dist*min_dist)
                        min_dist = (float) STBTT_sqrt(dist2);

                     if (num >= 1 && res[0] >= 0.0f && res[0] <= 1.0f) {
                        t = res[0], it = 1.0f - t;
                        px = it*it*x0 + 2*t*it*x1 + t*t*x2;
                        py = it*it*y0 + 2*t*it*y1 + t*t*y2;
                        dist2 = (px-sx)*(px-sx) + (py-sy)*(py-sy);
                        if (dist2 < min_dist * min_dist)
                           min_dist = (float) STBTT_sqrt(dist2);
                     }
                     if (num >= 2 && res[1] >= 0.0f && res[1] <= 1.0f) {
                        t = res[1], it = 1.0f - t;
                        px = it*it*x0 + 2*t*it*x1 + t*t*x2;
                        py = it*it*y0 + 2*t*it*y1 + t*t*y2;
                        dist2 = (px-sx)*(px-sx) + (py-sy)*(py-sy);
                        if (dist2 < min_dist * min_dist)
                           min_dist = (float) STBTT_sqrt(dist2);
                     }
                     if (num >= 3 && res[2] >= 0.0f && res[2] <= 1.0f) {
                        t = res[2], it = 1.0f - t;
                        px = it*it*x0 + 2*t*it*x1 + t*t*x2;
                        py = it*it*y0 + 2*t*it*y1 + t*t*y2;
                        dist2 = (px-sx)*(px-sx) + (py-sy)*(py-sy);
                        if (dist2 < min_dist * min_dist)
                           min_dist = (float) STBTT_sqrt(dist2);
                     }
                  }
               }
            }
            if (winding == 0)
               min_dist = -min_dist;  // if outside the shape, value is negative
            val = onedge_value + pixel_dist_scale * min_dist;
            if (val < 0)
               val = 0;
            else if (val > 255)
               val = 255;
            data[(y-iy0)*w+(x-ix0)] = (unsigned char) val;
         }
      }
      STBTT_free(precompute, info->userdata);
      STBTT_free(verts, info->userdata);
   }
   return data;
}

STBTT_DEF unsigned char * stbtt_GetCodepointSDF(const stbtt_fontinfo *info, float scale, int codepoint, int padding, unsigned char onedge_value, float pixel_dist_scale, int *width, int *height, int *xoff, int *yoff)
{
   return stbtt_GetGlyphSDF(info, scale, stbtt_FindGlyphIndex(info, codepoint), padding, onedge_value, pixel_dist_scale, width, height, xoff, yoff);
}

STBTT_DEF void stbtt_FreeSDF(unsigned char *bitmap, void *userdata)
{
   STBTT_free(bitmap, userdata);
}

//////////////////////////////////////////////////////////////////////////////
//
// font name matching -- recommended not to use this
//

// check if a utf8 string contains a prefix which is the utf16 string; if so return length of matching utf8 string
static stbtt_int32 stbtt__CompareUTF8toUTF16_bigendian_prefix(stbtt_uint8 *s1, stbtt_int32 len1, stbtt_uint8 *s2, stbtt_int32 len2)
{
   stbtt_int32 i=0;

   // convert utf16 to utf8 and compare the results while converting
   while (len2) {
      stbtt_uint16 ch = s2[0]*256 + s2[1];
      if (ch < 0x80) {
         if (i >= len1) return -1;
         if (s1[i++] != ch) return -1;
      } else if (ch < 0x800) {
         if (i+1 >= len1) return -1;
         if (s1[i++] != 0xc0 + (ch >> 6)) return -1;
         if (s1[i++] != 0x80 + (ch & 0x3f)) return -1;
      } else if (ch >= 0xd800 && ch < 0xdc00) {
         stbtt_uint32 c;
         stbtt_uint16 ch2 = s2[2]*256 + s2[3];
         if (i+3 >= len1) return -1;
         c = ((ch - 0xd800) << 10) + (ch2 - 0xdc00) + 0x10000;
         if (s1[i++] != 0xf0 + (c >> 18)) return -1;
         if (s1[i++] != 0x80 + ((c >> 12) & 0x3f)) return -1;
         if (s1[i++] != 0x80 + ((c >>  6) & 0x3f)) return -1;
         if (s1[i++] != 0x80 + ((c      ) & 0x3f)) return -1;
         s2 += 2; // plus another 2 below
         len2 -= 2;
      } else if (ch >= 0xdc00 && ch < 0xe000) {
         return -1;
      } else {
         if (i+2 >= len1) return -1;
         if (s1[i++] != 0xe0 + (ch >> 12)) return -1;
         if (s1[i++] != 0x80 + ((ch >> 6) & 0x3f)) return -1;
         if (s1[i++] != 0x80 + ((ch     ) & 0x3f)) return -1;
      }
      s2 += 2;
      len2 -= 2;
   }
   return i;
}

static int stbtt_CompareUTF8toUTF16_bigendian_internal(char *s1, int len1, char *s2, int len2)
{
   return len1 == stbtt__CompareUTF8toUTF16_bigendian_prefix((stbtt_uint8*) s1, len1, (stbtt_uint8*) s2, len2);
}

// returns results in whatever encoding you request... but note that 2-byte encodings
// will be BIG-ENDIAN... use stbtt_CompareUTF8toUTF16_bigendian() to compare
STBTT_DEF const char *stbtt_GetFontNameString(const stbtt_fontinfo *font, int *length, int platformID, int encodingID, int languageID, int nameID)
{
   stbtt_int32 i,count,stringOffset;
   stbtt_uint8 *fc = font->data;
   stbtt_uint32 offset = font->fontstart;
   stbtt_uint32 nm = stbtt__find_table(fc, offset, "name");
   if (!nm) return NULL;

   count = ttUSHORT(fc+nm+2);
   stringOffset = nm + ttUSHORT(fc+nm+4);
   for (i=0; i < count; ++i) {
      stbtt_uint32 loc = nm + 6 + 12 * i;
      if (platformID == ttUSHORT(fc+loc+0) && encodingID == ttUSHORT(fc+loc+2)
          && languageID == ttUSHORT(fc+loc+4) && nameID == ttUSHORT(fc+loc+6)) {
         *length = ttUSHORT(fc+loc+8);
         return (const char *) (fc+stringOffset+ttUSHORT(fc+loc+10));
      }
   }
   return NULL;
}

static int stbtt__matchpair(stbtt_uint8 *fc, stbtt_uint32 nm, stbtt_uint8 *name, stbtt_int32 nlen, stbtt_int32 target_id, stbtt_int32 next_id)
{
   stbtt_int32 i;
   stbtt_int32 count = ttUSHORT(fc+nm+2);
   stbtt_int32 stringOffset = nm + ttUSHORT(fc+nm+4);

   for (i=0; i < count; ++i) {
      stbtt_uint32 loc = nm + 6 + 12 * i;
      stbtt_int32 id = ttUSHORT(fc+loc+6);
      if (id == target_id) {
         // find the encoding
         stbtt_int32 platform = ttUSHORT(fc+loc+0), encoding = ttUSHORT(fc+loc+2), language = ttUSHORT(fc+loc+4);

         // is this a Unicode encoding?
         if (platform == 0 || (platform == 3 && encoding == 1) || (platform == 3 && encoding == 10)) {
            stbtt_int32 slen = ttUSHORT(fc+loc+8);
            stbtt_int32 off = ttUSHORT(fc+loc+10);

            // check if there's a prefix match
            stbtt_int32 matchlen = stbtt__CompareUTF8toUTF16_bigendian_prefix(name, nlen, fc+stringOffset+off,slen);
            if (matchlen >= 0) {
               // check for target_id+1 immediately following, with same encoding & language
               if (i+1 < count && ttUSHORT(fc+loc+12+6) == next_id && ttUSHORT(fc+loc+12) == platform && ttUSHORT(fc+loc+12+2) == encoding && ttUSHORT(fc+loc+12+4) == language) {
                  slen = ttUSHORT(fc+loc+12+8);
                  off = ttUSHORT(fc+loc+12+10);
                  if (slen == 0) {
                     if (matchlen == nlen)
                        return 1;
                  } else if (matchlen < nlen && name[matchlen] == ' ') {
                     ++matchlen;
                     if (stbtt_CompareUTF8toUTF16_bigendian_internal((char*) (name+matchlen), nlen-matchlen, (char*)(fc+stringOffset+off),slen))
                        return 1;
                  }
               } else {
                  // if nothing immediately following
                  if (matchlen == nlen)
                     return 1;
               }
            }
         }

         // @TODO handle other encodings
      }
   }
   return 0;
}

static int stbtt__matches(stbtt_uint8 *fc, stbtt_uint32 offset, stbtt_uint8 *name, stbtt_int32 flags)
{
   stbtt_int32 nlen = (stbtt_int32) STBTT_strlen((char *) name);
   stbtt_uint32 nm,hd;
   if (!stbtt__isfont(fc+offset)) return 0;

   // check italics/bold/underline flags in macStyle...
   if (flags) {
      hd = stbtt__find_table(fc, offset, "head");
      if ((ttUSHORT(fc+hd+44) & 7) != (flags & 7)) return 0;
   }

   nm = stbtt__find_table(fc, offset, "name");
   if (!nm) return 0;

   if (flags) {
      // if we checked the macStyle flags, then just check the family and ignore the subfamily
      if (stbtt__matchpair(fc, nm, name, nlen, 16, -1))  return 1;
      if (stbtt__matchpair(fc, nm, name, nlen,  1, -1))  return 1;
      if (stbtt__matchpair(fc, nm, name, nlen,  3, -1))  return 1;
   } else {
      if (stbtt__matchpair(fc, nm, name, nlen, 16, 17))  return 1;
      if (stbtt__matchpair(fc, nm, name, nlen,  1,  2))  return 1;
      if (stbtt__matchpair(fc, nm, name, nlen,  3, -1))  return 1;
   }

   return 0;
}

static int stbtt_FindMatchingFont_internal(unsigned char *font_collection, char *name_utf8, stbtt_int32 flags)
{
   stbtt_int32 i;
   for (i=0;;++i) {
      stbtt_int32 off = stbtt_GetFontOffsetForIndex(font_collection, i);
      if (off < 0) return off;
      if (stbtt__matches((stbtt_uint8 *) font_collection, off, (stbtt_uint8*) name_utf8, flags))
         return off;
   }
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

STBTT_DEF int stbtt_BakeFontBitmap(const unsigned char *data, int offset,
                                float pixel_height, unsigned char *pixels, int pw, int ph,
                                int first_char, int num_chars, stbtt_bakedchar *chardata)
{
   return stbtt_BakeFontBitmap_internal((unsigned char *) data, offset, pixel_height, pixels, pw, ph, first_char, num_chars, chardata);
}

STBTT_DEF int stbtt_GetFontOffsetForIndex(const unsigned char *data, int index)
{
   return stbtt_GetFontOffsetForIndex_internal((unsigned char *) data, index);
}

STBTT_DEF int stbtt_GetNumberOfFonts(const unsigned char *data)
{
   return stbtt_GetNumberOfFonts_internal((unsigned char *) data);
}

STBTT_DEF int stbtt_InitFont(stbtt_fontinfo *info, const unsigned char *data, int offset)
{
   return stbtt_InitFont_internal(info, (unsigned char *) data, offset);
}

STBTT_DEF int stbtt_FindMatchingFont(const unsigned char *fontdata, const char *name, int flags)
{
   return stbtt_FindMatchingFont_internal((unsigned char *) fontdata, (char *) name, flags);
}

STBTT_DEF int stbtt_CompareUTF8toUTF16_bigendian(const char *s1, int len1, const char *s2, int len2)
{
   return stbtt_CompareUTF8toUTF16_bigendian_internal((char *) s1, len1, (char *) s2, len2);
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif // STB_TRUETYPE_IMPLEMENTATION


// FULL VERSION HISTORY
//
//   1.25 (2021-07-11) many fixes
//   1.24 (2020-02-05) fix warning
//   1.23 (2020-02-02) query SVG data for glyphs; query whole kerning table (but only kern not GPOS)
//   1.22 (2019-08-11) minimize missing-glyph duplication; fix kerning if both 'GPOS' and 'kern' are defined
//   1.21 (2019-02-25) fix warning
//   1.20 (2019-02-07) PackFontRange skips missing codepoints; GetScaleFontVMetrics()
//   1.19 (2018-02-11) OpenType GPOS kerning (horizontal only), STBTT_fmod
//   1.18 (2018-01-29) add missing function
//   1.17 (2017-07-23) make more arguments const; doc fix
//   1.16 (2017-07-12) SDF support
//   1.15 (2017-03-03) make more arguments const
//   1.14 (2017-01-16) num-fonts-in-TTC function
//   1.13 (2017-01-02) support OpenType fonts, certain Apple fonts
//   1.12 (2016-10-25) suppress warnings about casting away const with -Wcast-qual
//   1.11 (2016-04-02) fix unused-variable warning
//   1.10 (2016-04-02) allow user-defined fabs() replacement
//                     fix memory leak if fontsize=0.0
//                     fix warning from duplicate typedef
//   1.09 (2016-01-16) warning fix; avoid crash on outofmem; use alloc userdata for PackFontRanges
//   1.08 (2015-09-13) document stbtt_Rasterize(); fixes for vertical & horizontal edges
//   1.07 (2015-08-01) allow PackFontRanges to accept arrays of sparse codepoints;
//                     allow PackFontRanges to pack and render in separate phases;
//                     fix stbtt_GetFontOFfsetForIndex (never worked for non-0 input?);
//                     fixed an assert() bug in the new rasterizer
//                     replace assert() with STBTT_assert() in new rasterizer
//   1.06 (2015-07-14) performance improvements (~35% faster on x86 and x64 on test machine)
//                     also more precise AA rasterizer, except if shapes overlap
//                     remove need for STBTT_sort
//   1.05 (2015-04-15) fix misplaced definitions for STBTT_STATIC
//   1.04 (2015-04-15) typo in example
//   1.03 (2015-04-12) STBTT_STATIC, fix memory leak in new packing, various fixes
//   1.02 (2014-12-10) fix various warnings & compile issues w/ stb_rect_pack, C++
//   1.01 (2014-12-08) fix subpixel position when oversampling to exactly match
//                        non-oversampled; STBTT_POINT_SIZE for packed case only
//   1.00 (2014-12-06) add new PackBegin etc. API, w/ support for oversampling
//   0.99 (2014-09-18) fix multiple bugs with subpixel rendering (ryg)
//   0.9  (2014-08-07) support certain mac/iOS fonts without an MS platformID
//   0.8b (2014-07-07) fix a warning
//   0.8  (2014-05-25) fix a few more warnings
//   0.7  (2013-09-25) bugfix: subpixel glyph bug fixed in 0.5 had come back
//   0.6c (2012-07-24) improve documentation
//   0.6b (2012-07-20) fix a few more warnings
//   0.6  (2012-07-17) fix warnings; added stbtt_ScaleForMappingEmToPixels,
//                        stbtt_GetFontBoundingBox, stbtt_IsGlyphEmpty
//   0.5  (2011-12-09) bugfixes:
//                        subpixel glyph renderer computed wrong bounding box
//                        first vertex of shape can be off-curve (FreeSans)
//   0.4b (2011-12-03) fixed an error in the font baking example
//   0.4  (2011-12-01) kerning, subpixel rendering (tor)
//                    bugfixes for:
//                        codepoint-to-glyph conversion using table fmt=12
//                        codepoint-to-glyph conversion using table fmt=4
//                        stbtt_GetBakedQuad with non-square texture (Zer)
//                    updated Hello World! sample to use kerning and subpixel
//                    fixed some warnings
//   0.3  (2009-06-24) cmap fmt=12, compound shapes (MM)
//                    userdata, malloc-from-userdata, non-zero fill (stb)
//   0.2  (2009-03-11) Fix unsigned/signed char warnings
//   0.1  (2009-03-09) First public release
//

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/

// End stb_truetype.h ----------------------------------------------------------
#endif // !__STB_INCLUDE_STB_TRUETYPE_H__ (was already defined)

// Clean up stb defines so they don't leak into user code.
// This prevents conflicts if the user includes stb_truetype.h after gut_single.h.
#ifdef GUT_OWNS_STB_TRUETYPE
#undef STB_TRUETYPE_IMPLEMENTATION
#undef GUT_OWNS_STB_TRUETYPE
#endif

// --- core/RefCounted.cpp ---


namespace gut {

// RefCounted implementation is header-only
// This file exists for potential future implementation needs

} // namespace gut


// --- core/Signal.cpp ---


namespace gut {

// Signal implementation is header-only (templates)
// This file exists for potential future non-template implementation needs

} // namespace gut


// --- core/Property.cpp ---


namespace gut {

PropertyRegistry& PropertyRegistry::instance() {
    static PropertyRegistry registry;
    return registry;
}

const PropertyMeta* PropertyRegistry::getMeta(PropertyId id) const {
    auto it = m_properties.find(id);
    return it != m_properties.end() ? &it->second : nullptr;
}

PropertyId PropertyRegistry::findByName(StringView name) const {
    auto it = m_nameToId.find(String(name));
    return it != m_nameToId.end() ? it->second : PropertyId{};
}

} // namespace gut


// --- core/Object.cpp ---


namespace gut {

// Object implementation is mostly inline/header-only
// This file exists for potential future implementation needs

} // namespace gut


// --- render/Geometry.cpp ---

#include <cmath>
#include <algorithm>

namespace gut {

void Path::moveTo(f32 x, f32 y) {
    m_segments.push_back(PathSegment::moveTo({x, y}));
    m_currentPoint = {x, y};
    updateBounds(m_currentPoint);
}

void Path::lineTo(f32 x, f32 y) {
    m_segments.push_back(PathSegment::lineTo({x, y}));
    m_currentPoint = {x, y};
    updateBounds(m_currentPoint);
}

void Path::quadTo(f32 cx, f32 cy, f32 x, f32 y) {
    m_segments.push_back(PathSegment::quadTo({cx, cy}, {x, y}));
    m_currentPoint = {x, y};
    updateBounds({cx, cy});
    updateBounds(m_currentPoint);
}

void Path::cubicTo(f32 c1x, f32 c1y, f32 c2x, f32 c2y, f32 x, f32 y) {
    m_segments.push_back(PathSegment::cubicTo({c1x, c1y}, {c2x, c2y}, {x, y}));
    m_currentPoint = {x, y};
    updateBounds({c1x, c1y});
    updateBounds({c2x, c2y});
    updateBounds(m_currentPoint);
}

void Path::arcTo(f32 rx, f32 ry, f32 rotation, bool largeArc, bool sweep, f32 x, f32 y) {
    PathSegment s;
    s.type = PathSegmentType::ArcTo;
    s.points[0] = {rx, ry};       // radii
    s.points[1] = {rotation, static_cast<f32>(largeArc)}; // rotation and largeArc flag
    s.points[2] = {x, y};         // end point
    s.param = sweep ? 1.0f : 0.0f;
    m_segments.push_back(s);
    m_currentPoint = {x, y};
    // Arc bounds calculation is complex; use endpoint for simple approximation
    updateBounds(m_currentPoint);
}

void Path::close() {
    m_segments.push_back(PathSegment::close());
}

void Path::clear() {
    m_segments.clear();
    m_currentPoint = {};
    m_bounds = {};
    m_boundsValid = false;
}

void Path::addRect(Rectf rect) {
    moveTo(rect.x, rect.y);
    lineTo(rect.x + rect.width, rect.y);
    lineTo(rect.x + rect.width, rect.y + rect.height);
    lineTo(rect.x, rect.y + rect.height);
    close();
}

void Path::addRoundedRect(Rectf rect, f32 cornerRadius) {
    addRoundedRect(rect, cornerRadius, cornerRadius, cornerRadius, cornerRadius);
}

void Path::addRoundedRect(Rectf rect, f32 topLeft, f32 topRight, f32 bottomRight, f32 bottomLeft) {
    f32 x = rect.x;
    f32 y = rect.y;
    f32 w = rect.width;
    f32 h = rect.height;
    
    // Clamp radii
    f32 maxRadius = std::min(w, h) * 0.5f;
    topLeft = std::min(topLeft, maxRadius);
    topRight = std::min(topRight, maxRadius);
    bottomRight = std::min(bottomRight, maxRadius);
    bottomLeft = std::min(bottomLeft, maxRadius);
    
    moveTo(x + topLeft, y);
    lineTo(x + w - topRight, y);
    if (topRight > 0) {
        arcTo(topRight, topRight, 0, false, true, x + w, y + topRight);
    }
    lineTo(x + w, y + h - bottomRight);
    if (bottomRight > 0) {
        arcTo(bottomRight, bottomRight, 0, false, true, x + w - bottomRight, y + h);
    }
    lineTo(x + bottomLeft, y + h);
    if (bottomLeft > 0) {
        arcTo(bottomLeft, bottomLeft, 0, false, true, x, y + h - bottomLeft);
    }
    lineTo(x, y + topLeft);
    if (topLeft > 0) {
        arcTo(topLeft, topLeft, 0, false, true, x + topLeft, y);
    }
    close();
}

void Path::addEllipse(Point2f center, f32 radiusX, f32 radiusY) {
    // Approximate ellipse with 4 cubic bezier curves
    constexpr f32 kappa = 0.5522847498f;
    
    f32 ox = radiusX * kappa;
    f32 oy = radiusY * kappa;
    f32 cx = center.x;
    f32 cy = center.y;
    
    moveTo(cx + radiusX, cy);
    cubicTo(cx + radiusX, cy + oy, cx + ox, cy + radiusY, cx, cy + radiusY);
    cubicTo(cx - ox, cy + radiusY, cx - radiusX, cy + oy, cx - radiusX, cy);
    cubicTo(cx - radiusX, cy - oy, cx - ox, cy - radiusY, cx, cy - radiusY);
    cubicTo(cx + ox, cy - radiusY, cx + radiusX, cy - oy, cx + radiusX, cy);
    close();
}

void Path::addCircle(Point2f center, f32 radius) {
    addEllipse(center, radius, radius);
}

void Path::updateBounds(Point2f point) {
    if (!m_boundsValid) {
        m_bounds = {point.x, point.y, 0, 0};
        m_boundsValid = true;
    } else {
        f32 minX = std::min(m_bounds.x, point.x);
        f32 minY = std::min(m_bounds.y, point.y);
        f32 maxX = std::max(m_bounds.x + m_bounds.width, point.x);
        f32 maxY = std::max(m_bounds.y + m_bounds.height, point.y);
        m_bounds = {minX, minY, maxX - minX, maxY - minY};
    }
}

bool Path::contains(Point2f point) const {
    // Simple even-odd rule implementation for closed paths
    if (m_segments.empty()) return false;
    
    int crossings = 0;
    Point2f prev{};
    Point2f start{};
    
    for (const auto& seg : m_segments) {
        switch (seg.type) {
            case PathSegmentType::MoveTo:
                prev = seg.points[0];
                start = prev;
                break;
                
            case PathSegmentType::LineTo: {
                Point2f curr = seg.points[0];
                // Ray casting from point to the right
                if ((prev.y <= point.y && curr.y > point.y) ||
                    (prev.y > point.y && curr.y <= point.y)) {
                    f32 t = (point.y - prev.y) / (curr.y - prev.y);
                    f32 intersectX = prev.x + t * (curr.x - prev.x);
                    if (point.x < intersectX) {
                        crossings++;
                    }
                }
                prev = curr;
                break;
            }
            
            case PathSegmentType::Close: {
                // Close segment acts like lineTo(start)
                Point2f curr = start;
                if ((prev.y <= point.y && curr.y > point.y) ||
                    (prev.y > point.y && curr.y <= point.y)) {
                    f32 t = (point.y - prev.y) / (curr.y - prev.y);
                    f32 intersectX = prev.x + t * (curr.x - prev.x);
                    if (point.x < intersectX) {
                        crossings++;
                    }
                }
                prev = curr;
                break;
            }
            
            default:
                // Simplified: treat curves as lines to endpoint
                prev = seg.points[seg.type == PathSegmentType::QuadTo ? 1 : 2];
                break;
        }
    }
    
    return (crossings % 2) == 1;
}

std::vector<std::vector<Point2f>> Path::flatten(f32 tolerance) const {
    std::vector<std::vector<Point2f>> subPaths;
    std::vector<Point2f> current;
    Point2f cursor{};
    Point2f startPt{};

    // Recursive subdivision helpers
    auto flattenQuad = [&](Point2f p0, Point2f cp, Point2f p1, f32 tol) {
        // de Casteljau subdivision
        struct Rec {
            std::vector<Point2f>& out;
            f32 tol;
            void subdivide(Point2f a, Point2f b, Point2f c, int depth) {
                f32 mx = (a.x + 2 * b.x + c.x) * 0.25f;
                f32 my = (a.y + 2 * b.y + c.y) * 0.25f;
                f32 dx = (a.x + c.x) * 0.5f - mx;
                f32 dy = (a.y + c.y) * 0.5f - my;
                if (depth > 8 || (dx * dx + dy * dy) < tol * tol) {
                    out.push_back(c);
                    return;
                }
                Point2f ab = {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f};
                Point2f bc = {(b.x + c.x) * 0.5f, (b.y + c.y) * 0.5f};
                Point2f abc = {(ab.x + bc.x) * 0.5f, (ab.y + bc.y) * 0.5f};
                subdivide(a, ab, abc, depth + 1);
                subdivide(abc, bc, c, depth + 1);
            }
        } rec{current, tol};
        rec.subdivide(p0, cp, p1, 0);
    };

    auto flattenCubic = [&](Point2f p0, Point2f c1, Point2f c2, Point2f p1, f32 tol) {
        struct Rec {
            std::vector<Point2f>& out;
            f32 tol;
            void subdivide(Point2f a, Point2f b, Point2f c, Point2f d, int depth) {
                // Flatness: max distance of control points from the chord a→d
                f32 ux = 3*b.x - 2*a.x - d.x; f32 uy = 3*b.y - 2*a.y - d.y;
                f32 vx = 3*c.x - 2*d.x - a.x; f32 vy = 3*c.y - 2*d.y - a.y;
                ux *= ux; uy *= uy; vx *= vx; vy *= vy;
                if (ux < vx) ux = vx; if (uy < vy) uy = vy;
                if (depth > 8 || (ux + uy) < tol * tol * 16) {
                    out.push_back(d);
                    return;
                }
                Point2f ab  = {(a.x+b.x)*0.5f, (a.y+b.y)*0.5f};
                Point2f bc  = {(b.x+c.x)*0.5f, (b.y+c.y)*0.5f};
                Point2f cd  = {(c.x+d.x)*0.5f, (c.y+d.y)*0.5f};
                Point2f abc = {(ab.x+bc.x)*0.5f, (ab.y+bc.y)*0.5f};
                Point2f bcd = {(bc.x+cd.x)*0.5f, (bc.y+cd.y)*0.5f};
                Point2f mid = {(abc.x+bcd.x)*0.5f, (abc.y+bcd.y)*0.5f};
                subdivide(a, ab, abc, mid, depth+1);
                subdivide(mid, bcd, cd, d, depth+1);
            }
        } rec{current, tol};
        rec.subdivide(p0, c1, c2, p1, 0);
    };

    for (const auto& seg : m_segments) {
        switch (seg.type) {
        case PathSegmentType::MoveTo:
            if (!current.empty()) subPaths.push_back(std::move(current));
            current.clear();
            cursor = seg.points[0];
            startPt = cursor;
            current.push_back(cursor);
            break;
        case PathSegmentType::LineTo:
            current.push_back(seg.points[0]);
            cursor = seg.points[0];
            break;
        case PathSegmentType::QuadTo:
            flattenQuad(cursor, seg.points[0], seg.points[1], tolerance);
            cursor = seg.points[1];
            break;
        case PathSegmentType::CubicTo:
            flattenCubic(cursor, seg.points[0], seg.points[1], seg.points[2], tolerance);
            cursor = seg.points[2];
            break;
        case PathSegmentType::ArcTo: {
            // Arc params are packed: points[0]={rx,ry}, points[1]={rotation,0},
            // points[2]={x,y=endpoint}, param = largeArc | (sweep<<1)
            f32 rx = std::abs(seg.points[0].x);
            f32 ry = std::abs(seg.points[0].y);
            Point2f endp = seg.points[2];
            if (rx < 0.001f || ry < 0.001f) {
                current.push_back(endp);
                cursor = endp;
                break;
            }
            // Approximate arc with line segments
            f32 dx = endp.x - cursor.x;
            f32 dy = endp.y - cursor.y;
            f32 dist = std::sqrt(dx*dx + dy*dy);
            i32 steps = std::max(4, (i32)(dist / tolerance));
            for (i32 i = 1; i <= steps; ++i) {
                f32 t = (f32)i / (f32)steps;
                current.push_back({cursor.x + dx*t, cursor.y + dy*t});
            }
            cursor = endp;
            break;
        }
        case PathSegmentType::Close:
            if (!current.empty() && (current.front().x != cursor.x || current.front().y != cursor.y))
                current.push_back(startPt);
            cursor = startPt;
            break;
        }
    }
    if (!current.empty()) subPaths.push_back(std::move(current));
    return subPaths;
}

} // namespace gut


// --- render/Brush.cpp ---


namespace gut {

SolidColorBrush::SolidColorBrush(Color c) {
    setColor(c);
}

LinearGradientBrush::LinearGradientBrush(Point2f start, Point2f end, std::vector<GradientStop> stops)
    : m_stops(std::move(stops))
{
    setStartPoint(start);
    setEndPoint(end);
}

RadialGradientBrush::RadialGradientBrush(Point2f c, f32 radius, std::vector<GradientStop> stops)
    : m_stops(std::move(stops))
{
    setCenter(c);
    setRadiusX(radius);
    setRadiusY(radius);
}

} // namespace gut


// --- render/Pen.cpp ---


namespace gut {

Pen::Pen(Ref<Brush> brush, f32 thickness)
    : m_brush(std::move(brush))
{
    setthickness(thickness);
    setlineCap(LineCap::Flat);
    setlineJoin(LineJoin::Miter);
    setmiterLimit(10.0f);
    setdashOffset(0.0f);
}

Pen::Pen(Color color, f32 thickness)
    : m_brush(makeRef<SolidColorBrush>(color))
{
    setthickness(thickness);
    setlineCap(LineCap::Flat);
    setlineJoin(LineJoin::Miter);
    setmiterLimit(10.0f);
    setdashOffset(0.0f);
}

} // namespace gut


// --- render/RenderBackend.cpp ---


namespace gut {

// RenderBackend and NullRenderBackend have inline implementations
// This file can be used for more complex backend implementations

} // namespace gut


// --- render/RenderContext.cpp ---

#include <cmath>

namespace gut {

namespace {
    constexpr f32 PI = 3.14159265358979323846f;
    
    u32 packColor(Color c, f32 opacity) {
        u8 r = static_cast<u8>(c.r * 255.0f);
        u8 g = static_cast<u8>(c.g * 255.0f);
        u8 b = static_cast<u8>(c.b * 255.0f);
        u8 a = static_cast<u8>(c.a * opacity * 255.0f);
        return (static_cast<u32>(a) << 24) | 
               (static_cast<u32>(b) << 16) | 
               (static_cast<u32>(g) << 8) | 
               static_cast<u32>(r);
    }
}

RenderContext::RenderContext(RenderBackend& backend) : m_backend(backend) {
    m_stateStack.push(State{});
    m_currentState = m_stateStack.top();
}

RenderContext::~RenderContext() = default;

void RenderContext::beginFrame(Size2f size, f32 devicePixelRatio) {
    m_frameSize = size;
    m_devicePixelRatio = devicePixelRatio;
    
    // Reset state
    while (m_stateStack.size() > 1) {
        m_stateStack.pop();
    }
    m_currentState = State{};
    
    // Clear buffers
    m_vertices.clear();
    m_indices.clear();
    m_commands.clear();
    
    m_backend.beginFrame(
        static_cast<u32>(size.width * devicePixelRatio),
        static_cast<u32>(size.height * devicePixelRatio),
        devicePixelRatio
    );
}

void RenderContext::endFrame() {
    flush();
    m_backend.endFrame();
}

void RenderContext::flush() {
    if (!m_commands.empty()) {
        m_backend.render(
            std::span<const Vertex>(m_vertices.data(), m_vertices.size()),
            std::span<const u32>(m_indices.data(), m_indices.size()),
            std::span<const DrawCommand>(m_commands.data(), m_commands.size())
        );
        m_vertices.clear();
        m_indices.clear();
        m_commands.clear();
    }
}

void RenderContext::save() {
    m_stateStack.push(m_currentState);
}

void RenderContext::restore() {
    if (m_stateStack.size() > 1) {
        m_currentState = m_stateStack.top();
        m_stateStack.pop();
    }
}

void RenderContext::translate(f32 x, f32 y) {
    // M = M * T(x,y)
    auto& m = m_currentState.m;
    m[2] += m[0] * x + m[1] * y;
    m[5] += m[3] * x + m[4] * y;
}

void RenderContext::scale(f32 sx, f32 sy) {
    // M = M * S(sx,sy)
    auto& m = m_currentState.m;
    m[0] *= sx; m[3] *= sx;
    m[1] *= sy; m[4] *= sy;
}

void RenderContext::rotate(f32 angle) {
    // M = M * R(angle)
    auto& m = m_currentState.m;
    f32 c = std::cos(angle), s = std::sin(angle);
    f32 a0 = m[0], a1 = m[1], a3 = m[3], a4 = m[4];
    m[0] = a0 * c + a1 * s;   m[1] = -a0 * s + a1 * c;
    m[3] = a3 * c + a4 * s;   m[4] = -a3 * s + a4 * c;
}

void RenderContext::skew(f32 skewX, f32 skewY) {
    // M = M * Sk(skewX, skewY) where Sk = [1 tan(skewX); tan(skewY) 1]
    auto& m = m_currentState.m;
    f32 tx = std::tan(skewX), ty = std::tan(skewY);
    f32 a0 = m[0], a1 = m[1], a3 = m[3], a4 = m[4];
    m[0] = a0 + a1 * ty;   m[1] = a0 * tx + a1;
    m[3] = a3 + a4 * ty;   m[4] = a3 * tx + a4;
}

Point2f RenderContext::transformPoint(Point2f point) const {
    const auto& m = m_currentState.m;
    return {
        m[0] * point.x + m[1] * point.y + m[2],
        m[3] * point.x + m[4] * point.y + m[5]
    };
}

void RenderContext::pushClip(Rectf rect, f32 cornerRadius) {
    // Transform all four corners and compute axis-aligned bounding box
    Point2f p0 = transformPoint({rect.x, rect.y});
    Point2f p1 = transformPoint({rect.x + rect.width, rect.y});
    Point2f p2 = transformPoint({rect.x + rect.width, rect.y + rect.height});
    Point2f p3 = transformPoint({rect.x, rect.y + rect.height});
    f32 minX = std::min({p0.x, p1.x, p2.x, p3.x});
    f32 minY = std::min({p0.y, p1.y, p2.y, p3.y});
    f32 maxX = std::max({p0.x, p1.x, p2.x, p3.x});
    f32 maxY = std::max({p0.y, p1.y, p2.y, p3.y});
    Rectf transformed = {minX, minY, maxX - minX, maxY - minY};
    
    if (m_currentState.hasClip && cornerRadius <= 0) {
        transformed = transformed.intersection(m_currentState.clipRect);
    }
    
    m_currentState.clipRect = transformed;
    m_currentState.clipCornerRadius = cornerRadius;
    m_currentState.hasClip = true;
    
    // Add clip command
    DrawCommand cmd;
    cmd.type = DrawCommandType::SetClip;
    cmd.clipRect = transformed;
    cmd.clipCornerRadius = cornerRadius;
    cmd.vertexOffset = 0;
    cmd.indexOffset = 0;
    cmd.indexCount = 0;
    m_commands.push_back(cmd);
}

void RenderContext::pushClip(const Path& path) {
    pushClip(path.bounds());
}

void RenderContext::popClip() {
    if (m_stateStack.size() > 1) {
        auto parent = m_stateStack.top();
        m_currentState.clipRect = parent.clipRect;
        m_currentState.clipCornerRadius = parent.clipCornerRadius;
        m_currentState.hasClip = parent.hasClip;
    } else {
        m_currentState.hasClip = false;
        m_currentState.clipCornerRadius = 0;
    }
    
    DrawCommand cmd;
    cmd.type = DrawCommandType::ClearClip;
    cmd.vertexOffset = 0;
    cmd.indexOffset = 0;
    cmd.indexCount = 0;
    m_commands.push_back(cmd);
}

void RenderContext::setOpacity(f32 opacity) {
    m_currentState.opacity = opacity;
}

f32 RenderContext::opacity() const {
    return m_currentState.opacity;
}

// Helper methods

u32 RenderContext::addVertices(const Vertex* verts, u32 count) {
    u32 offset = static_cast<u32>(m_vertices.size());
    m_vertices.insert(m_vertices.end(), verts, verts + count);
    return offset;
}

u32 RenderContext::addIndices(const u32* inds, u32 count, u32 vertexOffset) {
    u32 offset = static_cast<u32>(m_indices.size());
    for (u32 i = 0; i < count; ++i) {
        m_indices.push_back(inds[i] + vertexOffset);
    }
    return offset;
}

void RenderContext::addDrawCommand(DrawCommandType type, u32 indexCount, Texture* texture) {
    DrawCommand cmd;
    cmd.type = type;
    cmd.vertexOffset = 0;  // Not used with indexed drawing
    cmd.indexOffset = static_cast<u32>(m_indices.size()) - indexCount;
    cmd.indexCount = indexCount;
    cmd.clipRect = m_currentState.hasClip ? m_currentState.clipRect : Rectf{0, 0, m_frameSize.width, m_frameSize.height};
    cmd.texture = texture;
    m_commands.push_back(cmd);
}

void RenderContext::addRect(Rectf rect, Color color, Texture* texture, Rectf uvRect) {
    Point2f p0 = transformPoint({rect.x, rect.y});
    Point2f p1 = transformPoint({rect.x + rect.width, rect.y});
    Point2f p2 = transformPoint({rect.x + rect.width, rect.y + rect.height});
    Point2f p3 = transformPoint({rect.x, rect.y + rect.height});
    
    u32 c = packColor(color, m_currentState.opacity);
    
    Vertex verts[4] = {
        {p0.x, p0.y, uvRect.x, uvRect.y, c},
        {p1.x, p1.y, uvRect.x + uvRect.width, uvRect.y, c},
        {p2.x, p2.y, uvRect.x + uvRect.width, uvRect.y + uvRect.height, c},
        {p3.x, p3.y, uvRect.x, uvRect.y + uvRect.height, c}
    };
    
    u32 inds[6] = {0, 1, 2, 0, 2, 3};
    
    u32 vertOffset = addVertices(verts, 4);
    addIndices(inds, 6, vertOffset);
    addDrawCommand(texture ? DrawCommandType::DrawTexturedTriangles : DrawCommandType::DrawTriangles, 6, texture);
}

void RenderContext::addRoundedRect(Rectf rect, f32 radius, Color color) {
    // Clamp radius to half the smaller dimension
    radius = std::min(radius, std::min(rect.width, rect.height) / 2);
    
    if (radius <= 0) {
        addRect(rect, color);
        return;
    }
    
    const i32 cornerSegments = 8;
    u32 c = packColor(color, m_currentState.opacity);
    
    // Build vertices: center + corners
    std::vector<Vertex> verts;
    
    // Center vertex
    Point2f center = transformPoint({rect.x + rect.width / 2, rect.y + rect.height / 2});
    verts.push_back({center.x, center.y, 0.5f, 0.5f, c});
    
    // Corner centers
    Point2f corners[4] = {
        {rect.x + radius, rect.y + radius},                                 // top-left
        {rect.x + rect.width - radius, rect.y + radius},                    // top-right
        {rect.x + rect.width - radius, rect.y + rect.height - radius},      // bottom-right
        {rect.x + radius, rect.y + rect.height - radius}                    // bottom-left
    };
    
    f32 startAngles[4] = {PI, PI * 1.5f, 0, PI * 0.5f};
    
    for (i32 corner = 0; corner < 4; ++corner) {
        f32 startAngle = startAngles[corner];
        for (i32 i = 0; i <= cornerSegments; ++i) {
            f32 angle = startAngle + (PI / 2) * i / cornerSegments;
            f32 x = corners[corner].x + radius * std::cos(angle);
            f32 y = corners[corner].y + radius * std::sin(angle);
            Point2f p = transformPoint({x, y});
            verts.push_back({p.x, p.y, 0.5f, 0.5f, c});
        }
    }
    
    u32 vertOffset = addVertices(verts.data(), static_cast<u32>(verts.size()));
    
    // Build indices (triangle fan from center)
    std::vector<u32> inds;
    u32 numOuterVerts = static_cast<u32>(verts.size() - 1);
    for (u32 i = 0; i < numOuterVerts; ++i) {
        inds.push_back(0);  // center
        inds.push_back(1 + i);
        inds.push_back(1 + (i + 1) % numOuterVerts);
    }
    
    addIndices(inds.data(), static_cast<u32>(inds.size()), vertOffset);
    addDrawCommand(DrawCommandType::DrawTriangles, static_cast<u32>(inds.size()));
}

void RenderContext::addRectGradient(Rectf rect, Color topColor, Color bottomColor) {
    Point2f p0 = transformPoint({rect.x, rect.y});
    Point2f p1 = transformPoint({rect.x + rect.width, rect.y});
    Point2f p2 = transformPoint({rect.x + rect.width, rect.y + rect.height});
    Point2f p3 = transformPoint({rect.x, rect.y + rect.height});
    
    u32 cTop = packColor(topColor, m_currentState.opacity);
    u32 cBot = packColor(bottomColor, m_currentState.opacity);
    
    Vertex verts[4] = {
        {p0.x, p0.y, 0, 0, cTop},    // top-left
        {p1.x, p1.y, 1, 0, cTop},    // top-right
        {p2.x, p2.y, 1, 1, cBot},    // bottom-right
        {p3.x, p3.y, 0, 1, cBot}     // bottom-left
    };
    
    u32 inds[6] = {0, 1, 2, 0, 2, 3};
    
    u32 vertOffset = addVertices(verts, 4);
    addIndices(inds, 6, vertOffset);
    addDrawCommand(DrawCommandType::DrawTriangles, 6);
}

void RenderContext::addRoundedRectGradient(Rectf rect, f32 radius, Color topColor, Color bottomColor) {
    radius = std::min(radius, std::min(rect.width, rect.height) / 2);
    
    if (radius <= 0) {
        addRectGradient(rect, topColor, bottomColor);
        return;
    }
    
    const i32 cornerSegments = 8;
    f32 h = rect.height;
    
    // Build vertices: center + corners (each vertex colored by its Y position)
    std::vector<Vertex> verts;
    
    // Center vertex (at middle Y → blend 50%)
    Point2f center = transformPoint({rect.x + rect.width / 2, rect.y + h / 2});
    Color centerColor = sampleGradient(topColor, bottomColor, 0.5f);
    u32 cCenter = packColor(centerColor, m_currentState.opacity);
    verts.push_back({center.x, center.y, 0.5f, 0.5f, cCenter});
    
    // Corner centers
    Point2f corners[4] = {
        {rect.x + radius, rect.y + radius},                            // top-left
        {rect.x + rect.width - radius, rect.y + radius},               // top-right
        {rect.x + rect.width - radius, rect.y + h - radius},           // bottom-right
        {rect.x + radius, rect.y + h - radius}                         // bottom-left
    };
    
    f32 startAngles[4] = {PI, PI * 1.5f, 0, PI * 0.5f};
    
    for (i32 corner = 0; corner < 4; ++corner) {
        f32 startAngle = startAngles[corner];
        for (i32 i = 0; i <= cornerSegments; ++i) {
            f32 angle = startAngle + (PI / 2) * i / cornerSegments;
            f32 x = corners[corner].x + radius * std::cos(angle);
            f32 y = corners[corner].y + radius * std::sin(angle);
            
            // Color based on normalized Y position within the rect
            f32 t = (y - rect.y) / h;
            t = std::clamp(t, 0.0f, 1.0f);
            Color vertColor = sampleGradient(topColor, bottomColor, t);
            u32 c = packColor(vertColor, m_currentState.opacity);
            
            Point2f p = transformPoint({x, y});
            verts.push_back({p.x, p.y, 0.5f, 0.5f, c});
        }
    }
    
    u32 vertOffset = addVertices(verts.data(), static_cast<u32>(verts.size()));
    
    // Build indices (triangle fan from center)
    std::vector<u32> inds;
    u32 numOuterVerts = static_cast<u32>(verts.size() - 1);
    for (u32 i = 0; i < numOuterVerts; ++i) {
        inds.push_back(0);
        inds.push_back(1 + i);
        inds.push_back(1 + (i + 1) % numOuterVerts);
    }
    
    addIndices(inds.data(), static_cast<u32>(inds.size()), vertOffset);
    addDrawCommand(DrawCommandType::DrawTriangles, static_cast<u32>(inds.size()));
}

// ---------------------------------------------------------------------------
// Radial gradient mesh builders
// ---------------------------------------------------------------------------

void RenderContext::addRectRadialGradient(Rectf rect, const RadialGradientBrush& brush) {
    // We subdivide the rect into a grid and assign per-vertex colours from the
    // radial gradient.  A 16x16 grid is plenty for smooth colour fading.
    const auto& stops = brush.stops();
    if (stops.size() < 2) return;

    const i32 gridN = 16;  // cells per axis
    const i32 vertsPerSide = gridN + 1;

    // Gradient centre & radii are in [0,1] relative coords → map to rect.
    const f32 cx = rect.x + brush.center().x * rect.width;
    const f32 cy = rect.y + brush.center().y * rect.height;
    const f32 rx = brush.radiusX() * rect.width;
    const f32 ry = brush.radiusY() * rect.height;

    std::vector<Vertex> verts(vertsPerSide * vertsPerSide);
    for (i32 row = 0; row <= gridN; ++row) {
        for (i32 col = 0; col <= gridN; ++col) {
            f32 x = rect.x + rect.width  * col / gridN;
            f32 y = rect.y + rect.height * row / gridN;
            f32 dx = (rx > 0) ? (x - cx) / rx : 0;
            f32 dy = (ry > 0) ? (y - cy) / ry : 0;
            f32 dist = std::sqrt(dx * dx + dy * dy);
            dist = std::clamp(dist, 0.0f, 1.0f);
            Color c = sampleGradientStops(stops, dist);
            Point2f tp = transformPoint({x, y});
            verts[row * vertsPerSide + col] = {tp.x, tp.y, 0, 0, packColor(c, m_currentState.opacity)};
        }
    }

    std::vector<u32> inds;
    inds.reserve(gridN * gridN * 6);
    for (i32 row = 0; row < gridN; ++row) {
        for (i32 col = 0; col < gridN; ++col) {
            u32 tl = row * vertsPerSide + col;
            u32 tr = tl + 1;
            u32 bl = tl + vertsPerSide;
            u32 br = bl + 1;
            inds.push_back(tl); inds.push_back(tr); inds.push_back(br);
            inds.push_back(tl); inds.push_back(br); inds.push_back(bl);
        }
    }

    u32 vertOffset = addVertices(verts.data(), static_cast<u32>(verts.size()));
    addIndices(inds.data(), static_cast<u32>(inds.size()), vertOffset);
    addDrawCommand(DrawCommandType::DrawTriangles, static_cast<u32>(inds.size()));
}

void RenderContext::addRoundedRectRadialGradient(Rectf rect, f32 radius, const RadialGradientBrush& brush) {
    radius = std::min(radius, std::min(rect.width, rect.height) / 2);
    if (radius <= 0) {
        addRectRadialGradient(rect, brush);
        return;
    }

    const auto& stops = brush.stops();
    if (stops.size() < 2) return;

    const f32 cx = rect.x + brush.center().x * rect.width;
    const f32 cy = rect.y + brush.center().y * rect.height;
    const f32 rx = brush.radiusX() * rect.width;
    const f32 ry = brush.radiusY() * rect.height;

    auto radialColor = [&](f32 x, f32 y) -> u32 {
        f32 dx = (rx > 0) ? (x - cx) / rx : 0;
        f32 dy = (ry > 0) ? (y - cy) / ry : 0;
        f32 dist = std::clamp(std::sqrt(dx * dx + dy * dy), 0.0f, 1.0f);
        return packColor(sampleGradientStops(stops, dist), m_currentState.opacity);
    };

    // Build a triangle-fan rounded rect (center + boundary).
    constexpr i32 cornerSegs = 8;
    std::vector<Vertex> verts;

    // Center vertex
    f32 midX = rect.x + rect.width * 0.5f;
    f32 midY = rect.y + rect.height * 0.5f;
    Point2f ct = transformPoint({midX, midY});
    verts.push_back({ct.x, ct.y, 0.5f, 0.5f, radialColor(midX, midY)});

    Point2f corners[4] = {
        {rect.x + radius, rect.y + radius},
        {rect.x + rect.width - radius, rect.y + radius},
        {rect.x + rect.width - radius, rect.y + rect.height - radius},
        {rect.x + radius, rect.y + rect.height - radius}
    };
    f32 startAngles[4] = {PI, PI * 1.5f, 0, PI * 0.5f};

    for (i32 c = 0; c < 4; ++c) {
        for (i32 i = 0; i <= cornerSegs; ++i) {
            f32 angle = startAngles[c] + (PI / 2) * i / cornerSegs;
            f32 x = corners[c].x + radius * std::cos(angle);
            f32 y = corners[c].y + radius * std::sin(angle);
            Point2f tp = transformPoint({x, y});
            verts.push_back({tp.x, tp.y, 0, 0, radialColor(x, y)});
        }
    }

    u32 vertOffset = addVertices(verts.data(), static_cast<u32>(verts.size()));

    std::vector<u32> inds;
    u32 outerN = static_cast<u32>(verts.size() - 1);
    for (u32 i = 0; i < outerN; ++i) {
        inds.push_back(0);
        inds.push_back(1 + i);
        inds.push_back(1 + (i + 1) % outerN);
    }

    addIndices(inds.data(), static_cast<u32>(inds.size()), vertOffset);
    addDrawCommand(DrawCommandType::DrawTriangles, static_cast<u32>(inds.size()));
}

void RenderContext::addEllipseRadialGradient(Point2f center, f32 rx, f32 ry,
                                              const RadialGradientBrush& brush, i32 segments) {
    const auto& stops = brush.stops();
    if (stops.size() < 2) return;

    const f32 gcx = center.x + (brush.center().x - 0.5f) * rx * 2;
    const f32 gcy = center.y + (brush.center().y - 0.5f) * ry * 2;
    const f32 grx = brush.radiusX() * rx * 2;
    const f32 gry = brush.radiusY() * ry * 2;

    auto radialColor = [&](f32 x, f32 y) -> u32 {
        f32 dx = (grx > 0) ? (x - gcx) / grx : 0;
        f32 dy = (gry > 0) ? (y - gcy) / gry : 0;
        f32 dist = std::clamp(std::sqrt(dx * dx + dy * dy), 0.0f, 1.0f);
        return packColor(sampleGradientStops(stops, dist), m_currentState.opacity);
    };

    std::vector<Vertex> verts;
    Point2f ct = transformPoint(center);
    verts.push_back({ct.x, ct.y, 0.5f, 0.5f, radialColor(center.x, center.y)});

    for (i32 i = 0; i < segments; ++i) {
        f32 angle = 2 * PI * i / segments;
        f32 x = center.x + rx * std::cos(angle);
        f32 y = center.y + ry * std::sin(angle);
        Point2f tp = transformPoint({x, y});
        verts.push_back({tp.x, tp.y, 0, 0, radialColor(x, y)});
    }

    u32 vertOffset = addVertices(verts.data(), static_cast<u32>(verts.size()));

    std::vector<u32> inds;
    for (i32 i = 0; i < segments; ++i) {
        inds.push_back(0);
        inds.push_back(1 + i);
        inds.push_back(1 + (i + 1) % segments);
    }

    addIndices(inds.data(), static_cast<u32>(inds.size()), vertOffset);
    addDrawCommand(DrawCommandType::DrawTriangles, static_cast<u32>(inds.size()));
}

void RenderContext::addEllipse(Point2f center, f32 rx, f32 ry, Color color, i32 segments) {
    u32 c = packColor(color, m_currentState.opacity);
    
    std::vector<Vertex> verts;
    Point2f centerT = transformPoint(center);
    verts.push_back({centerT.x, centerT.y, 0.5f, 0.5f, c});
    
    for (i32 i = 0; i < segments; ++i) {
        f32 angle = 2 * PI * i / segments;
        f32 x = center.x + rx * std::cos(angle);
        f32 y = center.y + ry * std::sin(angle);
        Point2f p = transformPoint({x, y});
        verts.push_back({p.x, p.y, 0.5f, 0.5f, c});
    }
    
    u32 vertOffset = addVertices(verts.data(), static_cast<u32>(verts.size()));
    
    std::vector<u32> inds;
    for (i32 i = 0; i < segments; ++i) {
        inds.push_back(0);
        inds.push_back(1 + i);
        inds.push_back(1 + (i + 1) % segments);
    }
    
    addIndices(inds.data(), static_cast<u32>(inds.size()), vertOffset);
    addDrawCommand(DrawCommandType::DrawTriangles, static_cast<u32>(inds.size()));
}

void RenderContext::addLine(Point2f p1, Point2f p2, Color color, f32 thickness) {
    f32 dx = p2.x - p1.x;
    f32 dy = p2.y - p1.y;
    f32 len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.0001f) return;
    
    f32 nx = -dy / len * thickness / 2;
    f32 ny = dx / len * thickness / 2;
    
    u32 c = packColor(color, m_currentState.opacity);
    
    Point2f v0 = transformPoint({p1.x + nx, p1.y + ny});
    Point2f v1 = transformPoint({p1.x - nx, p1.y - ny});
    Point2f v2 = transformPoint({p2.x - nx, p2.y - ny});
    Point2f v3 = transformPoint({p2.x + nx, p2.y + ny});
    
    Vertex verts[4] = {
        {v0.x, v0.y, 0, 0, c},
        {v1.x, v1.y, 0, 1, c},
        {v2.x, v2.y, 1, 1, c},
        {v3.x, v3.y, 1, 0, c}
    };
    
    u32 inds[6] = {0, 1, 2, 0, 2, 3};
    
    u32 vertOffset = addVertices(verts, 4);
    addIndices(inds, 6, vertOffset);
    addDrawCommand(DrawCommandType::DrawTriangles, 6);
}

void RenderContext::addPolyline(const Point2f* pts, u32 count, Color color,
                                f32 thickness, bool closed, LineJoin join,
                                LineCap cap, f32 miterLimit) {
    if (count < 2) return;
    f32 halfW = thickness * 0.5f;
    u32 col = packColor(color, m_currentState.opacity);

    // Helper: perpendicular normal for segment a→b (left-hand side)
    auto segNormal = [](Point2f a, Point2f b) -> Point2f {
        f32 dx = b.x - a.x, dy = b.y - a.y;
        f32 len = std::sqrt(dx*dx + dy*dy);
        if (len < 1e-6f) return {0, 0};
        return {-dy / len, dx / len};
    };

    // Build offset contour points (left + right) for each vertex
    // For a polyline with N points we produce N "left" and N "right" positions.
    std::vector<Point2f> leftPts(count), rightPts(count);

    auto emitJoint = [&](u32 i, Point2f nPrev, Point2f nNext) {
        // Miter offset at joint between two segments
        f32 mx = nPrev.x + nNext.x;
        f32 my = nPrev.y + nNext.y;
        f32 mLen2 = mx*mx + my*my;
        if (mLen2 < 1e-10f) {
            leftPts[i]  = {pts[i].x + nPrev.x * halfW, pts[i].y + nPrev.y * halfW};
            rightPts[i] = {pts[i].x - nPrev.x * halfW, pts[i].y - nPrev.y * halfW};
            return;
        }
        // dot(miter, normal) — how far the miter extends
        f32 dot = mx * nPrev.x + my * nPrev.y;
        f32 miterLen = halfW / std::max(dot, 1e-6f);

        // Cap miter length
        if (join == LineJoin::Bevel || (join == LineJoin::Miter && miterLen > miterLimit * halfW)) {
            // Bevel — average of the two offset positions
            leftPts[i]  = {pts[i].x + (nPrev.x + nNext.x) * 0.5f * halfW,
                           pts[i].y + (nPrev.y + nNext.y) * 0.5f * halfW};
            rightPts[i] = {pts[i].x - (nPrev.x + nNext.x) * 0.5f * halfW,
                           pts[i].y - (nPrev.y + nNext.y) * 0.5f * halfW};
        } else {
            // Miter or Round (round approximated as miter for simplicity)
            f32 mNorm = 1.0f / std::sqrt(mLen2);
            f32 nmx = mx * mNorm, nmy = my * mNorm;
            leftPts[i]  = {pts[i].x + nmx * miterLen, pts[i].y + nmy * miterLen};
            rightPts[i] = {pts[i].x - nmx * miterLen, pts[i].y - nmy * miterLen};
        }
    };

    // Compute per-segment normals
    std::vector<Point2f> normals(count - 1);
    for (u32 i = 0; i + 1 < count; ++i) {
        normals[i] = segNormal(pts[i], pts[i + 1]);
    }

    if (closed && count >= 3) {
        // For closed polyline, every vertex has two adjacent segments
        Point2f nLast = segNormal(pts[count - 1], pts[0]); // wrapping segment
        // Treat the closing segment + first segment as a joint
        // We also need the segment from last-1 to last
        for (u32 i = 0; i < count; ++i) {
            Point2f nPrev = (i == 0) ? nLast : normals[i - 1];
            Point2f nNext = (i < count - 1) ? normals[i] : nLast;
            emitJoint(i, nPrev, nNext);
        }
    } else {
        // Start cap
        {
            Point2f n = normals[0];
            if (cap == LineCap::Square) {
                // Extend start backward by halfW
                f32 dx = pts[1].x - pts[0].x, dy = pts[1].y - pts[0].y;
                f32 len = std::sqrt(dx*dx + dy*dy);
                f32 tx = 0, ty = 0;
                if (len > 1e-6f) { tx = -dx/len * halfW; ty = -dy/len * halfW; }
                leftPts[0]  = {pts[0].x + n.x * halfW + tx, pts[0].y + n.y * halfW + ty};
                rightPts[0] = {pts[0].x - n.x * halfW + tx, pts[0].y - n.y * halfW + ty};
            } else {
                leftPts[0]  = {pts[0].x + n.x * halfW, pts[0].y + n.y * halfW};
                rightPts[0] = {pts[0].x - n.x * halfW, pts[0].y - n.y * halfW};
            }
        }
        // Interior joints
        for (u32 i = 1; i + 1 < count; ++i) {
            emitJoint(i, normals[i - 1], normals[i]);
        }
        // End cap
        {
            Point2f n = normals[count - 2];
            u32 e = count - 1;
            if (cap == LineCap::Square) {
                f32 dx = pts[e].x - pts[e-1].x, dy = pts[e].y - pts[e-1].y;
                f32 len = std::sqrt(dx*dx + dy*dy);
                f32 tx = 0, ty = 0;
                if (len > 1e-6f) { tx = dx/len * halfW; ty = dy/len * halfW; }
                leftPts[e]  = {pts[e].x + n.x * halfW + tx, pts[e].y + n.y * halfW + ty};
                rightPts[e] = {pts[e].x - n.x * halfW + tx, pts[e].y - n.y * halfW + ty};
            } else {
                leftPts[e]  = {pts[e].x + n.x * halfW, pts[e].y + n.y * halfW};
                rightPts[e] = {pts[e].x - n.x * halfW, pts[e].y - n.y * halfW};
            }
        }
    }

    // Emit triangle strip: for each segment i→i+1, we have a quad
    // (left[i], right[i], right[i+1], left[i+1])
    u32 segCount = closed ? count : (count - 1);
    std::vector<Vertex> verts(segCount * 4);
    std::vector<u32> inds(segCount * 6);

    for (u32 i = 0; i < segCount; ++i) {
        u32 j = (i + 1) % count;
        Point2f tl = transformPoint(leftPts[i]);
        Point2f tr = transformPoint(rightPts[i]);
        Point2f br = transformPoint(rightPts[j]);
        Point2f bl = transformPoint(leftPts[j]);
        u32 vi = i * 4;
        verts[vi + 0] = {tl.x, tl.y, 0, 0, col};
        verts[vi + 1] = {tr.x, tr.y, 0, 1, col};
        verts[vi + 2] = {br.x, br.y, 1, 1, col};
        verts[vi + 3] = {bl.x, bl.y, 1, 0, col};
        u32 ii = i * 6;
        inds[ii + 0] = vi; inds[ii + 1] = vi + 1; inds[ii + 2] = vi + 2;
        inds[ii + 3] = vi; inds[ii + 4] = vi + 2; inds[ii + 5] = vi + 3;
    }

    u32 vertOffset = addVertices(verts.data(), static_cast<u32>(verts.size()));
    addIndices(inds.data(), static_cast<u32>(inds.size()), vertOffset);
    addDrawCommand(DrawCommandType::DrawTriangles, static_cast<u32>(inds.size()));

    // Round caps: emit triangle fans at endpoints
    if (!closed && cap == LineCap::Round) {
        const i32 capSegs = 8;
        auto emitRoundCap = [&](Point2f center, Point2f leftP, Point2f rightP) {
            std::vector<Vertex> cv(capSegs + 2);
            std::vector<u32> ci(capSegs * 3);
            Point2f tc = transformPoint(center);
            cv[0] = {tc.x, tc.y, 0.5f, 0.5f, col};
            // Arc from leftP to rightP through 180 degrees
            f32 ax = leftP.x - center.x, ay = leftP.y - center.y;
            f32 bx = rightP.x - center.x, by = rightP.y - center.y;
            for (i32 s = 0; s <= capSegs; ++s) {
                f32 t = (f32)s / (f32)capSegs;
                f32 cosA = std::cos(PI * t);
                f32 sinA = std::sin(PI * t);
                // Rotate the left-offset vector
                f32 px = center.x + ax * cosA - ay * sinA;
                f32 py = center.y + ax * sinA + ay * cosA;
                Point2f tp = transformPoint({px, py});
                cv[1 + s] = {tp.x, tp.y, 0, 0, col};
            }
            for (i32 s = 0; s < capSegs; ++s) {
                ci[s*3 + 0] = 0;
                ci[s*3 + 1] = 1 + s;
                ci[s*3 + 2] = 2 + s;
            }
            u32 vo = addVertices(cv.data(), static_cast<u32>(cv.size()));
            addIndices(ci.data(), static_cast<u32>(ci.size()), vo);
            addDrawCommand(DrawCommandType::DrawTriangles, static_cast<u32>(ci.size()));
        };
        emitRoundCap(pts[0], leftPts[0], rightPts[0]);
        emitRoundCap(pts[count-1], rightPts[count-1], leftPts[count-1]);
    }
}

void RenderContext::addFilledPolygon(const Point2f* pts, u32 count, Color color) {
    // Ear-clipping triangulation
    if (count < 3) return;
    u32 col = packColor(color, m_currentState.opacity);

    // Build vertex list
    std::vector<Vertex> verts(count);
    for (u32 i = 0; i < count; ++i) {
        Point2f tp = transformPoint(pts[i]);
        verts[i] = {tp.x, tp.y, 0, 0, col};
    }
    u32 vertOffset = addVertices(verts.data(), count);

    // Ear clipping on untransformed polygon
    std::vector<u32> remaining(count);
    for (u32 i = 0; i < count; ++i) remaining[i] = i;

    auto cross2d = [](Point2f a, Point2f b, Point2f c) -> f32 {
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    };

    auto pointInTriangle = [&](Point2f p, Point2f a, Point2f b, Point2f c) -> bool {
        f32 d1 = cross2d(a, b, p);
        f32 d2 = cross2d(b, c, p);
        f32 d3 = cross2d(c, a, p);
        bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
        return !(hasNeg && hasPos);
    };

    // Determine winding
    f32 area = 0;
    for (u32 i = 0; i < count; ++i) {
        u32 j = (i + 1) % count;
        area += pts[i].x * pts[j].y - pts[j].x * pts[i].y;
    }
    bool ccw = area > 0;

    std::vector<u32> indices;
    indices.reserve((count - 2) * 3);

    while (remaining.size() > 2) {
        bool earFound = false;
        u32 n = static_cast<u32>(remaining.size());
        for (u32 i = 0; i < n; ++i) {
            u32 prev = remaining[(i + n - 1) % n];
            u32 curr = remaining[i];
            u32 next = remaining[(i + 1) % n];

            f32 c = cross2d(pts[prev], pts[curr], pts[next]);
            bool isConvex = ccw ? (c > 0) : (c < 0);
            if (!isConvex) continue;

            // Check no other vertex inside this triangle
            bool isEar = true;
            for (u32 j = 0; j < n; ++j) {
                u32 vi = remaining[j];
                if (vi == prev || vi == curr || vi == next) continue;
                if (pointInTriangle(pts[vi], pts[prev], pts[curr], pts[next])) {
                    isEar = false;
                    break;
                }
            }
            if (isEar) {
                indices.push_back(prev);
                indices.push_back(curr);
                indices.push_back(next);
                remaining.erase(remaining.begin() + i);
                earFound = true;
                break;
            }
        }
        if (!earFound) break; // Degenerate
    }

    if (!indices.empty()) {
        addIndices(indices.data(), static_cast<u32>(indices.size()), vertOffset);
        addDrawCommand(DrawCommandType::DrawTriangles, static_cast<u32>(indices.size()));
    }
}

// Drawing primitives

void RenderContext::fillTriangle(Point2f p1, Point2f p2, Point2f p3, Color color) {
    Point2f t0 = transformPoint(p1);
    Point2f t1 = transformPoint(p2);
    Point2f t2 = transformPoint(p3);
    u32 c = packColor(color, m_currentState.opacity);
    Vertex verts[3] = {
        {t0.x, t0.y, 0, 0, c},
        {t1.x, t1.y, 0, 0, c},
        {t2.x, t2.y, 0, 0, c}
    };
    u32 inds[3] = {0, 1, 2};
    u32 vertOffset = addVertices(verts, 3);
    addIndices(inds, 3, vertOffset);
    addDrawCommand(DrawCommandType::DrawTriangles, 3);
}

void RenderContext::fillRect(Rectf rect, Color color) {
    addRect(rect, color);
}

void RenderContext::fillRect(Rectf rect, const Brush& brush) {
    if (brush.type() == BrushType::Solid) {
        auto& solid = static_cast<const SolidColorBrush&>(brush);
        fillRect(rect, solid.color());
    } else if (brush.type() == BrushType::LinearGradient) {
        auto& grad = static_cast<const LinearGradientBrush&>(brush);
        if (grad.stops().size() >= 2) {
            fillRectGradient(rect, grad.stops().front().color, grad.stops().back().color);
        }
    } else if (brush.type() == BrushType::RadialGradient) {
        auto& rad = static_cast<const RadialGradientBrush&>(brush);
        if (rad.stops().size() >= 2) {
            addRectRadialGradient(rect, rad);
        }
    }
}

void RenderContext::fillRectGradient(Rectf rect, Color topColor, Color bottomColor) {
    addRectGradient(rect, topColor, bottomColor);
}

void RenderContext::strokeRect(Rectf rect, Color color, f32 thickness) {
    f32 t = thickness;
    // Top
    addRect({rect.x, rect.y, rect.width, t}, color);
    // Bottom
    addRect({rect.x, rect.y + rect.height - t, rect.width, t}, color);
    // Left
    addRect({rect.x, rect.y + t, t, rect.height - 2 * t}, color);
    // Right
    addRect({rect.x + rect.width - t, rect.y + t, t, rect.height - 2 * t}, color);
}

void RenderContext::strokeRect(Rectf rect, const Pen& pen) {
    if (pen.brush() && pen.brush()->type() == BrushType::Solid) {
        auto& solid = static_cast<const SolidColorBrush&>(*pen.brush());
        strokeRect(rect, solid.color(), pen.thickness());
    }
}

void RenderContext::fillRoundedRect(Rectf rect, f32 cornerRadius, Color color) {
    addRoundedRect(rect, cornerRadius, color);
}

void RenderContext::fillRoundedRect(Rectf rect, f32 cornerRadius, const Brush& brush) {
    if (brush.type() == BrushType::Solid) {
        auto& solid = static_cast<const SolidColorBrush&>(brush);
        fillRoundedRect(rect, cornerRadius, solid.color());
    } else if (brush.type() == BrushType::LinearGradient) {
        auto& grad = static_cast<const LinearGradientBrush&>(brush);
        if (grad.stops().size() >= 2) {
            fillRoundedRectGradient(rect, cornerRadius, grad.stops().front().color, grad.stops().back().color);
        }
    } else if (brush.type() == BrushType::RadialGradient) {
        auto& rad = static_cast<const RadialGradientBrush&>(brush);
        if (rad.stops().size() >= 2) {
            addRoundedRectRadialGradient(rect, cornerRadius, rad);
        }
    }
}

void RenderContext::fillRoundedRectGradient(Rectf rect, f32 cornerRadius, Color topColor, Color bottomColor) {
    addRoundedRectGradient(rect, cornerRadius, topColor, bottomColor);
}

void RenderContext::drawDropShadow(Rectf rect, f32 cornerRadius, Color shadowColor,
                                    f32 blurRadius, f32 offsetX, f32 offsetY) {
    addDropShadow(rect, cornerRadius, shadowColor, blurRadius, offsetX, offsetY);
}

void RenderContext::addDropShadow(Rectf rect, f32 cornerRadius, Color sc,
                                   f32 blur, f32 ox, f32 oy) {
    f32 innerCr = std::min(cornerRadius, std::min(rect.width, rect.height) / 2);
    Rectf innerRect = {rect.x + ox, rect.y + oy, rect.width, rect.height};
    u32 cInner = packColor(sc, m_currentState.opacity);
    
    // Hard shadow (blur == 0): just a solid filled rounded rect at the offset
    if (blur <= 0) {
        fillRoundedRect(innerRect, innerCr, sc);
        return;
    }
    
    // Soft shadow: solid body + fade-out ring
    f32 outerCr = innerCr + blur;
    Rectf outerRect = {innerRect.x - blur, innerRect.y - blur,
                       innerRect.width + blur * 2, innerRect.height + blur * 2};
    
    Color outerColor(sc.r, sc.g, sc.b, 0.0f);
    u32 cOuter = packColor(outerColor, m_currentState.opacity);
    
    const i32 segs = 8;
    
    auto buildContour = [&](Rectf r, f32 cr) -> std::vector<Point2f> {
        std::vector<Point2f> pts;
        Point2f corners[4] = {
            {r.x + cr, r.y + cr},
            {r.x + r.width - cr, r.y + cr},
            {r.x + r.width - cr, r.y + r.height - cr},
            {r.x + cr, r.y + r.height - cr}
        };
        f32 startAngles[4] = {PI, PI * 1.5f, 0.0f, PI * 0.5f};
        for (i32 corner = 0; corner < 4; ++corner) {
            for (i32 i = 0; i <= segs; ++i) {
                f32 angle = startAngles[corner] + (PI * 0.5f) * static_cast<f32>(i) / static_cast<f32>(segs);
                pts.push_back({corners[corner].x + cr * std::cos(angle),
                               corners[corner].y + cr * std::sin(angle)});
            }
        }
        return pts;
    };
    
    auto innerPts = buildContour(innerRect, innerCr);
    auto outerPts = buildContour(outerRect, outerCr);
    u32 n = static_cast<u32>(innerPts.size());
    
    // Vertex layout:
    //   [0]        = center of inner rect (for solid fill fan)
    //   [1..n]     = inner contour (shadow color)
    //   [n+1..2n]  = outer contour (alpha = 0)
    std::vector<Vertex> verts;
    verts.reserve(1 + n * 2);
    
    // Center vertex for the solid fill
    Point2f center = transformPoint({innerRect.x + innerRect.width * 0.5f,
                                      innerRect.y + innerRect.height * 0.5f});
    verts.push_back({center.x, center.y, 0.5f, 0.5f, cInner});
    
    // Inner contour vertices
    for (u32 i = 0; i < n; ++i) {
        Point2f p = transformPoint(innerPts[i]);
        verts.push_back({p.x, p.y, 0.5f, 0.5f, cInner});
    }
    // Outer contour vertices
    for (u32 i = 0; i < n; ++i) {
        Point2f p = transformPoint(outerPts[i]);
        verts.push_back({p.x, p.y, 0.5f, 0.5f, cOuter});
    }
    
    std::vector<u32> inds;
    inds.reserve(n * 3 + n * 6);
    
    // Part 1: Triangle fan — solid shadow body
    for (u32 i = 0; i < n; ++i) {
        u32 next = (i + 1) % n;
        inds.push_back(0);           // center
        inds.push_back(1 + i);       // inner current
        inds.push_back(1 + next);    // inner next
    }
    
    // Part 2: Quad-strip ring — fade halo
    for (u32 i = 0; i < n; ++i) {
        u32 next = (i + 1) % n;
        u32 ii = 1 + i;             // inner current
        u32 oi = 1 + n + i;         // outer current
        u32 in_ = 1 + next;         // inner next
        u32 on_ = 1 + n + next;     // outer next
        inds.push_back(ii); inds.push_back(oi); inds.push_back(in_);
        inds.push_back(oi); inds.push_back(on_); inds.push_back(in_);
    }
    
    u32 vertOffset = addVertices(verts.data(), static_cast<u32>(verts.size()));
    addIndices(inds.data(), static_cast<u32>(inds.size()), vertOffset);
    addDrawCommand(DrawCommandType::DrawTriangles, static_cast<u32>(inds.size()));
}

void RenderContext::drawInsetShadow(Rectf rect, f32 cornerRadius, Color shadowColor,
                                     f32 blurRadius, f32 offsetX, f32 offsetY) {
    addInsetShadow(rect, cornerRadius, shadowColor, blurRadius, offsetX, offsetY);
}

void RenderContext::addInsetShadow(Rectf rect, f32 cornerRadius, Color sc,
                                    f32 blur, f32 ox, f32 oy) {
    f32 cr = std::min(cornerRadius, std::min(rect.width, rect.height) / 2);

    // The outer contour sits on the rect boundary (shifted by offset).
    // The inner contour is inset by the blur radius.
    // Shadow colour at the outer contour fades to alpha=0 at the inner contour.
    // This creates a soft glow around the inside edge of the rect.

    if (blur <= 0 && ox == 0 && oy == 0) return;  // nothing to draw
    f32 effectiveBlur = std::max(blur, 0.5f);

    // Outer contour = the element boundary (shifted by -offset so the shadow
    // "comes from" that direction).  We actually shift the outer rect and
    // clamp with the inner to get the asymmetric offset effect.
    Rectf outerRect = {rect.x + ox, rect.y + oy, rect.width, rect.height};
    f32 outerCr = cr;

    // Inner contour = outer shrunk inward by blur
    Rectf innerRect = {outerRect.x + effectiveBlur, outerRect.y + effectiveBlur,
                       outerRect.width - effectiveBlur * 2, outerRect.height - effectiveBlur * 2};
    f32 innerCr = std::max(0.0f, outerCr - effectiveBlur);

    // If inner rect has degenerated, clamp to a very thin rect
    if (innerRect.width < 0) { innerRect.x = outerRect.x + outerRect.width * 0.5f; innerRect.width = 0; }
    if (innerRect.height < 0) { innerRect.y = outerRect.y + outerRect.height * 0.5f; innerRect.height = 0; }

    const i32 segs = 8;
    u32 cShadow = packColor(sc, m_currentState.opacity);
    Color fadeColor(sc.r, sc.g, sc.b, 0.0f);
    u32 cFade = packColor(fadeColor, m_currentState.opacity);

    auto buildContour = [&](Rectf r, f32 rad) -> std::vector<Point2f> {
        std::vector<Point2f> pts;
        if (rad <= 0) {
            // Simple rectangle corners
            pts.push_back({r.x, r.y});
            pts.push_back({r.x + r.width, r.y});
            pts.push_back({r.x + r.width, r.y + r.height});
            pts.push_back({r.x, r.y + r.height});
            return pts;
        }
        Point2f corners[4] = {
            {r.x + rad, r.y + rad},
            {r.x + r.width - rad, r.y + rad},
            {r.x + r.width - rad, r.y + r.height - rad},
            {r.x + rad, r.y + r.height - rad}
        };
        f32 startAngles[4] = {PI, PI * 1.5f, 0.0f, PI * 0.5f};
        for (i32 corner = 0; corner < 4; ++corner) {
            for (i32 i = 0; i <= segs; ++i) {
                f32 angle = startAngles[corner] + (PI * 0.5f) * static_cast<f32>(i) / static_cast<f32>(segs);
                pts.push_back({corners[corner].x + rad * std::cos(angle),
                               corners[corner].y + rad * std::sin(angle)});
            }
        }
        return pts;
    };

    auto outerPts = buildContour(outerRect, outerCr);
    auto innerPts = buildContour(innerRect, innerCr);

    // The two contours may have different point counts if one is rounded and
    // the other degenerated to a rectangle.  Resample the shorter one.
    // For simplicity, if sizes differ we just use the outer contour and shrink
    // each point toward the rect centre to build the inner contour.
    u32 n = static_cast<u32>(outerPts.size());
    if (innerPts.size() != outerPts.size()) {
        // Rebuild inner by projecting outer points inward
        Point2f cen = {outerRect.x + outerRect.width * 0.5f, outerRect.y + outerRect.height * 0.5f};
        innerPts.resize(n);
        for (u32 i = 0; i < n; ++i) {
            f32 dx = outerPts[i].x - cen.x;
            f32 dy = outerPts[i].y - cen.y;
            f32 len = std::sqrt(dx * dx + dy * dy);
            if (len > 0) {
                f32 shrink = std::min(effectiveBlur, len);
                innerPts[i] = {outerPts[i].x - dx / len * shrink,
                               outerPts[i].y - dy / len * shrink};
            } else {
                innerPts[i] = outerPts[i];
            }
        }
    }

    // Vertices: outer contour (shadow colour) + inner contour (alpha=0)
    std::vector<Vertex> verts;
    verts.reserve(n * 2);

    for (u32 i = 0; i < n; ++i) {
        Point2f p = transformPoint(outerPts[i]);
        verts.push_back({p.x, p.y, 0.5f, 0.5f, cShadow});
    }
    for (u32 i = 0; i < n; ++i) {
        Point2f p = transformPoint(innerPts[i]);
        verts.push_back({p.x, p.y, 0.5f, 0.5f, cFade});
    }

    // Quad-strip ring: outer[i]–inner[i]–outer[i+1]–inner[i+1]
    std::vector<u32> inds;
    inds.reserve(n * 6);
    for (u32 i = 0; i < n; ++i) {
        u32 next = (i + 1) % n;
        u32 oi = i;              // outer current
        u32 ii = n + i;          // inner current
        u32 on_ = next;          // outer next
        u32 in_ = n + next;      // inner next
        inds.push_back(oi); inds.push_back(ii); inds.push_back(on_);
        inds.push_back(ii); inds.push_back(in_); inds.push_back(on_);
    }

    // We need to clip this ring to the element rect so it doesn't bleed outside.
    pushClip(rect, cr);

    u32 vertOffset = addVertices(verts.data(), static_cast<u32>(verts.size()));
    addIndices(inds.data(), static_cast<u32>(inds.size()), vertOffset);
    addDrawCommand(DrawCommandType::DrawTriangles, static_cast<u32>(inds.size()));

    popClip();
}

void RenderContext::fillRoundedRect(Rectf rect, f32 topLeft, f32 topRight, f32 bottomRight, f32 bottomLeft, const Brush& brush) {
    // Simplified: use average radius
    f32 avgRadius = (topLeft + topRight + bottomRight + bottomLeft) / 4;
    fillRoundedRect(rect, avgRadius, brush);
}

void RenderContext::strokeRoundedRect(Rectf rect, f32 cornerRadius, Color color, f32 thickness) {
    cornerRadius = std::min(cornerRadius, std::min(rect.width, rect.height) / 2);
    
    if (cornerRadius <= 0) {
        strokeRect(rect, color, thickness);
        return;
    }
    
    const i32 cornerSegments = 8;
    u32 c = packColor(color, m_currentState.opacity);
    f32 t = thickness;
    f32 innerRadius = std::max(cornerRadius - t, 0.0f);
    
    // Outer corner centers
    Point2f outerCorners[4] = {
        {rect.x + cornerRadius, rect.y + cornerRadius},
        {rect.x + rect.width - cornerRadius, rect.y + cornerRadius},
        {rect.x + rect.width - cornerRadius, rect.y + rect.height - cornerRadius},
        {rect.x + cornerRadius, rect.y + rect.height - cornerRadius}
    };
    
    // Inner corner centers (inset by thickness)
    Point2f innerCorners[4] = {
        {rect.x + t + innerRadius - (innerRadius > 0 ? 0 : 0), rect.y + t + innerRadius - (innerRadius > 0 ? 0 : 0)},
        {rect.x + rect.width - t - innerRadius, rect.y + t + innerRadius},
        {rect.x + rect.width - t - innerRadius, rect.y + rect.height - t - innerRadius},
        {rect.x + t + innerRadius, rect.y + rect.height - t - innerRadius}
    };
    // Simplify: inner corners track outer corners
    for (int i = 0; i < 4; i++) innerCorners[i] = outerCorners[i];
    
    f32 startAngles[4] = {PI, PI * 1.5f, 0, PI * 0.5f};
    
    // Generate outer and inner ring vertices
    std::vector<Vertex> verts;
    for (i32 corner = 0; corner < 4; ++corner) {
        for (i32 i = 0; i <= cornerSegments; ++i) {
            f32 angle = startAngles[corner] + (PI / 2) * i / cornerSegments;
            f32 cosA = std::cos(angle);
            f32 sinA = std::sin(angle);
            
            // Outer vertex
            Point2f op = transformPoint({outerCorners[corner].x + cornerRadius * cosA,
                                         outerCorners[corner].y + cornerRadius * sinA});
            verts.push_back({op.x, op.y, 0.5f, 0.5f, c});
            
            // Inner vertex
            Point2f ip = transformPoint({innerCorners[corner].x + innerRadius * cosA,
                                         innerCorners[corner].y + innerRadius * sinA});
            verts.push_back({ip.x, ip.y, 0.5f, 0.5f, c});
        }
    }
    
    u32 vertOffset = addVertices(verts.data(), static_cast<u32>(verts.size()));
    
    // Build quad strip indices (outer[i], inner[i], outer[i+1], inner[i+1])
    u32 numPairs = static_cast<u32>(verts.size() / 2);
    std::vector<u32> inds;
    for (u32 i = 0; i < numPairs; ++i) {
        u32 next = (i + 1) % numPairs;
        u32 o0 = i * 2, i0 = i * 2 + 1;
        u32 o1 = next * 2, i1 = next * 2 + 1;
        inds.push_back(o0); inds.push_back(i0); inds.push_back(o1);
        inds.push_back(i0); inds.push_back(i1); inds.push_back(o1);
    }
    
    addIndices(inds.data(), static_cast<u32>(inds.size()), vertOffset);
    addDrawCommand(DrawCommandType::DrawTriangles, static_cast<u32>(inds.size()));
}

void RenderContext::strokeRoundedRect(Rectf rect, f32 cornerRadius, const Pen& pen) {
    if (pen.brush() && pen.brush()->type() == BrushType::Solid) {
        auto& solid = static_cast<const SolidColorBrush&>(*pen.brush());
        strokeRoundedRect(rect, cornerRadius, solid.color(), pen.thickness());
    }
}

void RenderContext::fillEllipse(Point2f center, f32 radiusX, f32 radiusY, Color color) {
    addEllipse(center, radiusX, radiusY, color);
}

void RenderContext::fillEllipse(Point2f center, f32 radiusX, f32 radiusY, const Brush& brush) {
    if (brush.type() == BrushType::Solid) {
        auto& solid = static_cast<const SolidColorBrush&>(brush);
        fillEllipse(center, radiusX, radiusY, solid.color());
    } else if (brush.type() == BrushType::RadialGradient) {
        auto& rad = static_cast<const RadialGradientBrush&>(brush);
        if (rad.stops().size() >= 2) {
            addEllipseRadialGradient(center, radiusX, radiusY, rad);
        }
    }
}

void RenderContext::strokeEllipse(Point2f center, f32 radiusX, f32 radiusY, Color color, f32 thickness) {
    // Draw as series of line segments
    const i32 segments = 32;
    for (i32 i = 0; i < segments; ++i) {
        f32 a1 = 2 * PI * i / segments;
        f32 a2 = 2 * PI * (i + 1) / segments;
        Point2f p1 = {center.x + radiusX * std::cos(a1), center.y + radiusY * std::sin(a1)};
        Point2f p2 = {center.x + radiusX * std::cos(a2), center.y + radiusY * std::sin(a2)};
        addLine(p1, p2, color, thickness);
    }
}

void RenderContext::strokeEllipse(Point2f center, f32 radiusX, f32 radiusY, const Pen& pen) {
    if (pen.brush() && pen.brush()->type() == BrushType::Solid) {
        auto& solid = static_cast<const SolidColorBrush&>(*pen.brush());
        strokeEllipse(center, radiusX, radiusY, solid.color(), pen.thickness());
    }
}

void RenderContext::fillCircle(Point2f center, f32 radius, Color color) {
    fillEllipse(center, radius, radius, color);
}

void RenderContext::fillCircle(Point2f center, f32 radius, const Brush& brush) {
    fillEllipse(center, radius, radius, brush);
}

void RenderContext::strokeCircle(Point2f center, f32 radius, Color color, f32 thickness) {
    strokeEllipse(center, radius, radius, color, thickness);
}

void RenderContext::strokeCircle(Point2f center, f32 radius, const Pen& pen) {
    strokeEllipse(center, radius, radius, pen);
}

void RenderContext::drawLine(Point2f p1, Point2f p2, Color color, f32 thickness) {
    addLine(p1, p2, color, thickness);
}

void RenderContext::drawLine(Point2f p1, Point2f p2, const Pen& pen) {
    if (pen.brush() && pen.brush()->type() == BrushType::Solid) {
        auto& solid = static_cast<const SolidColorBrush&>(*pen.brush());
        drawLine(p1, p2, solid.color(), pen.thickness());
    }
}

// --- Polylines ---

void RenderContext::drawPolyline(const Point2f* points, u32 count, Color color,
                                 f32 thickness, bool closed, LineJoin join,
                                 LineCap cap, f32 miterLimit) {
    addPolyline(points, count, color, thickness, closed, join, cap, miterLimit);
}

void RenderContext::drawPolyline(const std::vector<Point2f>& pts, Color color,
                                 f32 thickness, bool closed, LineJoin join,
                                 LineCap cap, f32 miterLimit) {
    drawPolyline(pts.data(), static_cast<u32>(pts.size()), color, thickness, closed, join, cap, miterLimit);
}

void RenderContext::drawPolyline(const Point2f* points, u32 count, const Pen& pen, bool closed) {
    if (pen.brush() && pen.brush()->type() == BrushType::Solid) {
        auto& solid = static_cast<const SolidColorBrush&>(*pen.brush());
        drawPolyline(points, count, solid.color(), pen.thickness(), closed,
                     pen.lineJoin(), pen.lineCap(), pen.miterLimit());
    }
}

void RenderContext::fillPolygon(const Point2f* points, u32 count, Color color) {
    addFilledPolygon(points, count, color);
}

void RenderContext::fillPolygon(const std::vector<Point2f>& pts, Color color) {
    fillPolygon(pts.data(), static_cast<u32>(pts.size()), color);
}

// --- Path rendering ---

void RenderContext::fillPath(const Path& path, Color color) {
    auto subPaths = path.flatten(0.5f);
    for (auto& sp : subPaths) {
        if (sp.size() >= 3)
            addFilledPolygon(sp.data(), static_cast<u32>(sp.size()), color);
    }
}

void RenderContext::fillPath(const Path& path, const Brush& brush) {
    if (brush.type() == BrushType::Solid) {
        auto& solid = static_cast<const SolidColorBrush&>(brush);
        fillPath(path, solid.color());
    }
}

void RenderContext::strokePath(const Path& path, Color color, f32 thickness) {
    auto subPaths = path.flatten(0.5f);
    for (auto& sp : subPaths) {
        if (sp.size() < 2) continue;
        // Detect if sub-path is closed (first == last)
        bool closed = (sp.size() >= 3 &&
                       std::abs(sp.front().x - sp.back().x) < 0.01f &&
                       std::abs(sp.front().y - sp.back().y) < 0.01f);
        if (closed) sp.pop_back(); // remove duplicate closing point
        addPolyline(sp.data(), static_cast<u32>(sp.size()), color, thickness,
                    closed, LineJoin::Miter, LineCap::Flat, 10.0f);
    }
}

void RenderContext::strokePath(const Path& path, const Pen& pen) {
    if (pen.brush() && pen.brush()->type() == BrushType::Solid) {
        auto& solid = static_cast<const SolidColorBrush&>(*pen.brush());
        auto subPaths = path.flatten(0.5f);
        for (auto& sp : subPaths) {
            if (sp.size() < 2) continue;
            bool closed = (sp.size() >= 3 &&
                           std::abs(sp.front().x - sp.back().x) < 0.01f &&
                           std::abs(sp.front().y - sp.back().y) < 0.01f);
            if (closed) sp.pop_back();
            addPolyline(sp.data(), static_cast<u32>(sp.size()), solid.color(),
                        pen.thickness(), closed, pen.lineJoin(), pen.lineCap(),
                        pen.miterLimit());
        }
    }
}

void RenderContext::drawImage(const Texture& texture, Rectf destRect) {
    drawImage(texture, destRect, Color::white());
}

void RenderContext::drawImage(const Texture& texture, Rectf destRect, Color tint) {
    Rectf srcRect = {0, 0, static_cast<f32>(texture.width()), static_cast<f32>(texture.height())};
    drawImage(texture, srcRect, destRect, tint);
}

void RenderContext::drawImage(const Texture& texture, Rectf srcRect, Rectf destRect) {
    drawImage(texture, srcRect, destRect, Color::white());
}

void RenderContext::drawImage(const Texture& texture, Rectf srcRect, Rectf destRect, Color tint) {
    // Convert src rect to UV coordinates
    f32 texW = static_cast<f32>(texture.width());
    f32 texH = static_cast<f32>(texture.height());
    Rectf uvRect = {
        srcRect.x / texW,
        srcRect.y / texH,
        srcRect.width / texW,
        srcRect.height / texH
    };
    
    addRect(destRect, tint, const_cast<Texture*>(&texture), uvRect);
}

void RenderContext::drawImageNineSlice(const Texture& texture, Thickness borders,
                                       Rectf destRect, Color tint) {
    Rectf srcRect = {0, 0, static_cast<f32>(texture.width()), static_cast<f32>(texture.height())};
    drawImageNineSlice(texture, srcRect, borders, destRect, tint);
}

void RenderContext::drawImageNineSlice(const Texture& texture, Rectf srcRect,
                                       Thickness borders, Rectf destRect,
                                       Color tint) {
    f32 texW = static_cast<f32>(texture.width());
    f32 texH = static_cast<f32>(texture.height());
    Texture* tex = const_cast<Texture*>(&texture);
    
    // Clamp borders so they don't exceed source or dest dimensions
    f32 bL = std::min(borders.left,   std::min(srcRect.width, destRect.width) / 2);
    f32 bR = std::min(borders.right,  std::min(srcRect.width, destRect.width) / 2);
    f32 bT = std::min(borders.top,    std::min(srcRect.height, destRect.height) / 2);
    f32 bB = std::min(borders.bottom, std::min(srcRect.height, destRect.height) / 2);
    
    // Source x/y splits (in pixels)
    f32 sx[4] = { srcRect.x, srcRect.x + bL, srcRect.x + srcRect.width - bR, srcRect.x + srcRect.width };
    f32 sy[4] = { srcRect.y, srcRect.y + bT, srcRect.y + srcRect.height - bB, srcRect.y + srcRect.height };
    
    // Destination x/y splits
    f32 dx[4] = { destRect.x, destRect.x + bL, destRect.x + destRect.width - bR, destRect.x + destRect.width };
    f32 dy[4] = { destRect.y, destRect.y + bT, destRect.y + destRect.height - bB, destRect.y + destRect.height };
    
    // Emit 9 quads (3 rows × 3 columns)
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            f32 dw = dx[col + 1] - dx[col];
            f32 dh = dy[row + 1] - dy[row];
            if (dw <= 0 || dh <= 0) continue; // degenerate slice, skip
            
            Rectf dest = { dx[col], dy[row], dw, dh };
            Rectf uv = {
                sx[col]           / texW,
                sy[row]           / texH,
                (sx[col + 1] - sx[col]) / texW,
                (sy[row + 1] - sy[row]) / texH
            };
            addRect(dest, tint, tex, uv);
        }
    }
}

Size2f RenderContext::measureText(const TextLayout& layout) {
    return m_backend.measureText(layout);
}

void RenderContext::drawText(const TextLayout& layout, Point2f position, Color color) {
    // Text rendering: backend generates vertices into our buffers
    Point2f transformedPos = transformPoint(position);
    
    std::vector<Vertex> textVerts;
    std::vector<u32> textIndices;
    m_backend.renderText(layout, transformedPos, color, textVerts, textIndices);
    
    if (!textVerts.empty() && !textIndices.empty()) {
        u32 vertOffset = addVertices(textVerts.data(), static_cast<u32>(textVerts.size()));
        addIndices(textIndices.data(), static_cast<u32>(textIndices.size()), vertOffset);
        
        // Text uses font atlas texture
        Texture* atlas = m_backend.fontAtlas();
        addDrawCommand(atlas ? DrawCommandType::DrawTexturedTriangles : DrawCommandType::DrawTriangles,
                       static_cast<u32>(textIndices.size()), atlas);
    }
}

void RenderContext::drawText(FontFace* face, const std::string& text, Point2f position, Color color) {
    if (!face || text.empty()) return;
    
    // Ensure atlas texture is created (lazy upload)
    if (face->needsUpload() && face->atlasBitmap() && face->atlasWidth() > 0) {
        auto texture = m_backend.createAlphaTexture(
            static_cast<u32>(face->atlasWidth()),
            static_cast<u32>(face->atlasHeight()),
            face->atlasBitmap()
        );
        if (texture) {
            // Store the texture in the face (need const_cast since face is const-ish)
            // This is a bit awkward but acceptable for lazy initialization
            const_cast<FontFace*>(face)->m_atlas = std::move(texture);
            const_cast<FontFace*>(face)->markUploaded();
        }
    }
    
    Texture* atlasTexture = face->atlasTexture();
    Point2f transformedPos = transformPoint(position);
    u32 packedColor = packColor(color, m_currentState.opacity);
    
    f32 x = transformedPos.x;
    f32 y = transformedPos.y;
    u32 prevCodepoint = 0;
    
    // Generate vertex/index data for each glyph
    std::vector<Vertex> verts;
    std::vector<u32> inds;
    verts.reserve(text.length() * 4);
    inds.reserve(text.length() * 6);
    
    for (char c : text) {
        // Handle newlines
        if (c == '\n') {
            x = transformedPos.x;
            y += face->lineHeight();
            prevCodepoint = 0;
            continue;
        }
        
        u32 codepoint = static_cast<u32>(static_cast<unsigned char>(c));
        const Glyph* g = face->glyph(codepoint);
        
        if (!g || g->width <= 0 || g->height <= 0) {
            // Skip missing glyphs but still advance
            if (g) {
                x += g->advance;
                if (prevCodepoint != 0) {
                    x += face->kerning(prevCodepoint, codepoint);
                }
            }
            prevCodepoint = codepoint;
            continue;
        }
        
        // Apply kerning
        if (prevCodepoint != 0) {
            x += face->kerning(prevCodepoint, codepoint);
        }
        
        // Position: baseline-relative
        f32 gx = x + g->bearingX;
        f32 gy = y - g->bearingY; // bearingY is distance from baseline to top
        
        // Generate quad
        u32 baseIndex = static_cast<u32>(verts.size());
        
        Vertex tl, tr, bl, br;
        
        // Top-left
        tl.x = gx;
        tl.y = gy;
        tl.u = g->u0;
        tl.v = g->v0;
        tl.color = packedColor;
        
        // Top-right
        tr.x = gx + g->width;
        tr.y = gy;
        tr.u = g->u1;
        tr.v = g->v0;
        tr.color = packedColor;
        
        // Bottom-left
        bl.x = gx;
        bl.y = gy + g->height;
        bl.u = g->u0;
        bl.v = g->v1;
        bl.color = packedColor;
        
        // Bottom-right
        br.x = gx + g->width;
        br.y = gy + g->height;
        br.u = g->u1;
        br.v = g->v1;
        br.color = packedColor;
        
        verts.push_back(tl);
        verts.push_back(tr);
        verts.push_back(bl);
        verts.push_back(br);
        
        // Two triangles: TL-TR-BL, TR-BR-BL
        inds.push_back(baseIndex + 0);
        inds.push_back(baseIndex + 1);
        inds.push_back(baseIndex + 2);
        inds.push_back(baseIndex + 1);
        inds.push_back(baseIndex + 3);
        inds.push_back(baseIndex + 2);
        
        x += g->advance;
        prevCodepoint = codepoint;
    }
    
    if (!verts.empty() && !inds.empty()) {
        u32 vertOffset = addVertices(verts.data(), static_cast<u32>(verts.size()));
        addIndices(inds.data(), static_cast<u32>(inds.size()), vertOffset);
        
        addDrawCommand(atlasTexture ? DrawCommandType::DrawTexturedTriangles : DrawCommandType::DrawTriangles,
                       static_cast<u32>(inds.size()), atlasTexture);
    }
}

void RenderContext::drawTextMultiline(FontFace* face, const std::string& text, Point2f position, Color color,
                                      f32 maxWidth, TextAlignment alignment) {
    if (!face || text.empty()) return;
    
    // Ensure atlas texture is uploaded
    if (face->needsUpload() && face->atlasBitmap() && face->atlasWidth() > 0) {
        auto texture = m_backend.createAlphaTexture(
            static_cast<u32>(face->atlasWidth()),
            static_cast<u32>(face->atlasHeight()),
            face->atlasBitmap()
        );
        if (texture) {
            const_cast<FontFace*>(face)->m_atlas = std::move(texture);
            const_cast<FontFace*>(face)->markUploaded();
        }
    }
    
    Texture* atlasTexture = face->atlasTexture();
    u32 packedColor = packColor(color, m_currentState.opacity);
    
    // Use TextShaper for word wrapping + alignment
    TextShaper shaper;
    auto lines = shaper.shapeMultiline(face, text, maxWidth, alignment);
    
    std::vector<Vertex> verts;
    std::vector<u32> inds;
    verts.reserve(text.length() * 4);
    inds.reserve(text.length() * 6);
    
    f32 lineY = 0;
    for (const auto& line : lines) {
        for (const auto& sg : line.glyphs) {
            const Glyph* g = sg.glyph;
            if (!g || g->width <= 0 || g->height <= 0) continue;
            
            Point2f glyphPos = transformPoint({
                position.x + sg.x,
                position.y + lineY - g->bearingY
            });
            
            u32 baseIndex = static_cast<u32>(verts.size());
            
            Vertex tl, tr, bl, br;
            tl.x = glyphPos.x;              tl.y = glyphPos.y;
            tl.u = g->u0;                   tl.v = g->v0;
            tl.color = packedColor;
            
            tr.x = glyphPos.x + g->width;   tr.y = glyphPos.y;
            tr.u = g->u1;                   tr.v = g->v0;
            tr.color = packedColor;
            
            bl.x = glyphPos.x;              bl.y = glyphPos.y + g->height;
            bl.u = g->u0;                   bl.v = g->v1;
            bl.color = packedColor;
            
            br.x = glyphPos.x + g->width;   br.y = glyphPos.y + g->height;
            br.u = g->u1;                   br.v = g->v1;
            br.color = packedColor;
            
            verts.push_back(tl);
            verts.push_back(tr);
            verts.push_back(bl);
            verts.push_back(br);
            
            inds.push_back(baseIndex + 0);
            inds.push_back(baseIndex + 1);
            inds.push_back(baseIndex + 2);
            inds.push_back(baseIndex + 1);
            inds.push_back(baseIndex + 3);
            inds.push_back(baseIndex + 2);
        }
        lineY += face->lineHeight();
    }
    
    if (!verts.empty() && !inds.empty()) {
        u32 vertOffset = addVertices(verts.data(), static_cast<u32>(verts.size()));
        addIndices(inds.data(), static_cast<u32>(inds.size()), vertOffset);
        addDrawCommand(atlasTexture ? DrawCommandType::DrawTexturedTriangles : DrawCommandType::DrawTriangles,
                       static_cast<u32>(inds.size()), atlasTexture);
    }
}

void RenderContext::drawBackdropBlur(Rectf rect, f32 cornerRadius, f32 blurRadius, Color tint) {
    if (blurRadius <= 0) return;
    
    // Transform rect to screen coordinates
    Point2f topLeft = transformPoint({rect.x, rect.y});
    Point2f bottomRight = transformPoint({rect.x + rect.width, rect.y + rect.height});
    
    // Flush any pending draw commands before the blur
    flush();
    
    // Emit a backdrop blur command
    DrawCommand cmd{};
    cmd.type = DrawCommandType::DrawBackdropBlur;
    cmd.blurRect = {topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y};
    cmd.blurRadius = blurRadius;
    cmd.blurCornerRadius = cornerRadius;
    cmd.blurTint = tint;
    m_commands.push_back(cmd);
}

} // namespace gut


// --- input/KeyEvent.cpp ---


namespace gut {

// KeyEvent is a simple struct, no implementation needed

} // namespace gut


// --- input/MouseEvent.cpp ---


namespace gut {

// MouseEvent is a simple struct, no implementation needed

} // namespace gut


// --- input/FocusManager.cpp ---


namespace gut {

FocusManager::FocusManager(Context& context) : m_context(context) {}

FocusManager::~FocusManager() = default;

void FocusManager::setFocus(Element* element) {
    if (element == m_focusedElement) {
        return;
    }
    
    // Check if element is focusable
    if (element && !element->focusable()) {
        return;
    }
    
    Element* oldFocus = m_focusedElement;
    
    if (m_focusedElement) {
        m_focusedElement->unfocus();
    }
    
    m_focusedElement = element;
    
    if (m_focusedElement) {
        m_focusedElement->focus();
    }
    
    m_focusChanged.emit(oldFocus, m_focusedElement);
}

void FocusManager::clearFocus() {
    setFocus(nullptr);
}

void FocusManager::moveFocus(FocusDirection direction) {
    Element* next = findFocusableInDirection(m_focusedElement, direction);
    if (next) {
        setFocus(next);
    }
}

void FocusManager::focusNext() {
    Element* next = findNextFocusable(m_focusedElement, true);
    if (next) {
        setFocus(next);
    }
}

void FocusManager::focusPrevious() {
    Element* prev = findNextFocusable(m_focusedElement, false);
    if (prev) {
        setFocus(prev);
    }
}

Element* FocusManager::findFocusableInDirection(Element* current, FocusDirection direction) {
    if (direction == FocusDirection::Next) {
        return findNextFocusable(current, true);
    } else if (direction == FocusDirection::Previous) {
        return findNextFocusable(current, false);
    }
    
    // TODO: Implement directional focus navigation (Up, Down, Left, Right)
    // This requires comparing element positions
    
    return nullptr;
}

void FocusManager::collectFocusableElements(Element* root, std::vector<Element*>& result) {
    if (!root) return;
    
    if (root->focusable() && root->isEnabled() && root->visibility() == Visibility::Visible) {
        result.push_back(root);
    }
    
    for (usize i = 0; i < root->childCount(); ++i) {
        collectFocusableElements(root->childAt(i), result);
    }
}

Element* FocusManager::findNextFocusable(Element* current, bool forward) {
    std::vector<Element*> focusable;
    collectFocusableElements(m_context.root(), focusable);
    
    if (focusable.empty()) {
        return nullptr;
    }
    
    // Sort by tabIndex (stable sort preserves tree order for equal tabIndex)
    std::stable_sort(focusable.begin(), focusable.end(),
                     [](const Element* a, const Element* b) {
                         return a->tabIndex() < b->tabIndex();
                     });
    
    // Find current element in list
    isize currentIndex = -1;
    for (usize i = 0; i < focusable.size(); ++i) {
        if (focusable[i] == current) {
            currentIndex = static_cast<isize>(i);
            break;
        }
    }
    
    // Find next/previous
    if (forward) {
        isize nextIndex = (currentIndex + 1) % static_cast<isize>(focusable.size());
        return focusable[static_cast<usize>(nextIndex)];
    } else {
        isize prevIndex = currentIndex - 1;
        if (prevIndex < 0) prevIndex = static_cast<isize>(focusable.size()) - 1;
        return focusable[static_cast<usize>(prevIndex)];
    }
}

} // namespace gut


// --- input/InputManager.cpp ---

#include <chrono>

namespace gut {

InputManager::InputManager(Context& context) : m_context(context) {}

InputManager::~InputManager() = default;

void InputManager::processMouseMove(f32 x, f32 y) {
    Point2f oldPos = m_mousePosition;
    m_mousePosition = {x, y};
    
    updateHoveredElement();
    
    // Send move event (bubbles up the parent chain)
    Element* target = m_capturedElement ? m_capturedElement : m_hoveredElement;
    if (target) {
        MouseEvent event = createMouseEvent(MouseEventType::Move);
        event.delta = {x - oldPos.x, y - oldPos.y};
        dispatchMouseEvent(target, event);
    }
}

void InputManager::processMouseButton(MouseButton button, bool pressed) {
    u32 buttonBit = 1u << static_cast<u32>(button);
    
    if (pressed) {
        m_mouseButtonState |= buttonBit;
        
        // Check for double-click
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        u64 nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
        
        f32 distance = std::sqrt(
            (m_mousePosition.x - m_lastClickPosition.x) * (m_mousePosition.x - m_lastClickPosition.x) +
            (m_mousePosition.y - m_lastClickPosition.y) * (m_mousePosition.y - m_lastClickPosition.y)
        );
        
        if (button == m_lastClickButton && 
            nowMs - m_lastClickTime < m_doubleClickInterval &&
            distance < m_doubleClickDistance) {
            m_clickCount++;
        } else {
            m_clickCount = 1;
        }
        
        m_lastClickTime = nowMs;
        m_lastClickPosition = m_mousePosition;
        m_lastClickButton = button;
        
        // Send button down event (bubbles up the parent chain)
        Element* target = m_capturedElement ? m_capturedElement : m_hoveredElement;
        if (target) {
            MouseEvent event = createMouseEvent(MouseEventType::ButtonDown, button);
            event.clickCount = m_clickCount;
            dispatchMouseEvent(target, event);
            
            m_pressedElement = target;
            
            // Handle double-click
            if (m_clickCount == 2) {
                MouseEvent dblEvent = createMouseEvent(MouseEventType::DoubleClick, button);
                dispatchMouseEvent(target, dblEvent);
            }
        }
    } else {
        m_mouseButtonState &= ~buttonBit;
        
        Element* target = m_capturedElement ? m_capturedElement : m_hoveredElement;
        if (target) {
            MouseEvent event = createMouseEvent(MouseEventType::ButtonUp, button);
            dispatchMouseEvent(target, event);
        }
        
        m_pressedElement = nullptr;
    }
}

void InputManager::processMouseWheel(f32 deltaX, f32 deltaY) {
    Element* target = m_capturedElement ? m_capturedElement : m_hoveredElement;
    if (target) {
        MouseEvent event = createMouseEvent(MouseEventType::Wheel);
        event.delta = {deltaX, deltaY};
        dispatchMouseEvent(target, event);
    }
}

void InputManager::processKey(Key key, bool pressed, ModifierKeys mods) {
    m_modifiers = mods;
    
    u32 keyIndex = static_cast<u32>(key);
    u32 wordIndex = keyIndex / 64;
    u64 bitMask = 1ull << (keyIndex % 64);
    
    if (wordIndex < 4) {
        if (pressed) {
            m_keyState[wordIndex] |= bitMask;
        } else {
            m_keyState[wordIndex] &= ~bitMask;
        }
    }
    
    Element* target = m_context.focusManager().focusedElement();
    if (target) {
        KeyEvent event = createKeyEvent(pressed ? KeyEventType::KeyDown : KeyEventType::KeyUp, key);
        dispatchKeyEvent(target, event);

        // If Tab wasn't consumed by the focused element, do focus navigation
        if (pressed && key == Key::Tab && !event.handled) {
            if (hasModifier(mods, ModifierKeys::Shift)) {
                m_context.focusManager().focusPrevious();
            } else {
                m_context.focusManager().focusNext();
            }
        }
    } else if (pressed && key == Key::Tab) {
        // Nothing focused — Tab focuses the first focusable element
        m_context.focusManager().focusNext();
    }
}

void InputManager::processTextInput(char32_t character) {
    Element* target = m_context.focusManager().focusedElement();
    if (target) {
        KeyEvent event;
        event.type = KeyEventType::Char;
        event.character = character;
        event.modifiers = m_modifiers;
        dispatchKeyEvent(target, event);
    }
}

bool InputManager::isMouseButtonDown(MouseButton button) const {
    u32 buttonBit = 1u << static_cast<u32>(button);
    return (m_mouseButtonState & buttonBit) != 0;
}

bool InputManager::isKeyDown(Key key) const {
    u32 keyIndex = static_cast<u32>(key);
    u32 wordIndex = keyIndex / 64;
    u64 bitMask = 1ull << (keyIndex % 64);
    
    return wordIndex < 4 && (m_keyState[wordIndex] & bitMask) != 0;
}

void InputManager::captureMouse(Element* element) {
    m_capturedElement = element;
}

void InputManager::releaseMouse() {
    m_capturedElement = nullptr;
}

void InputManager::updateHoveredElement() {
    Element* root = m_context.root();
    if (!root) {
        if (m_hoveredElement) {
            // Walk up old chain, leave all ancestors
            Element* current = m_hoveredElement;
            while (current) {
                current->onMouseLeave();
                current = current->parent();
            }
            m_hoveredElement = nullptr;
        }
        return;
    }
    
    // Convert screen coordinates to root-local coordinates
    Point2f localPoint = {
        m_mousePosition.x - root->bounds().x,
        m_mousePosition.y - root->bounds().y
    };
    
    Element* newHovered = root->hitTest(localPoint);
    
    if (newHovered != m_hoveredElement) {
        // Build ancestor chains (element → parent → grandparent → ...)
        auto buildChain = [](Element* e) -> std::vector<Element*> {
            std::vector<Element*> chain;
            while (e) {
                chain.push_back(e);
                e = e->parent();
            }
            return chain;
        };
        
        auto contains = [](const std::vector<Element*>& chain, Element* e) {
            return std::find(chain.begin(), chain.end(), e) != chain.end();
        };
        
        auto oldChain = buildChain(m_hoveredElement);
        auto newChain = buildChain(newHovered);
        
        // Leave old elements that are NOT ancestors of the new hovered element
        for (Element* e : oldChain) {
            if (!contains(newChain, e)) {
                e->onMouseLeave();
            }
        }
        
        // Enter new elements that were NOT ancestors of the old hovered element
        for (Element* e : newChain) {
            if (!contains(oldChain, e)) {
                e->onMouseEnter();
            }
        }
        
        m_hoveredElement = newHovered;
    }

    // Resolve cursor from the effective target (captured or hovered)
    Element* cursorTarget = m_capturedElement ? m_capturedElement : m_hoveredElement;
    CursorType resolvedCursor = CursorType::Arrow;
    if (cursorTarget) {
        resolvedCursor = cursorTarget->cursor();
    }
    m_context.notifyCursorChanged(resolvedCursor);
}

MouseEvent InputManager::createMouseEvent(MouseEventType type, MouseButton button) {
    MouseEvent event;
    event.type = type;
    event.screenPosition = m_mousePosition;
    event.position = m_mousePosition;  // Will be converted to local coords by dispatchMouseEvent
    event.button = button;
    event.modifiers = m_modifiers;
    return event;
}

KeyEvent InputManager::createKeyEvent(KeyEventType type, Key key) {
    KeyEvent event;
    event.type = type;
    event.key = key;
    event.modifiers = m_modifiers;
    return event;
}

void InputManager::dispatchMouseEvent(Element* target, MouseEvent event) {
    if (!target) return;
    
    // Enter/Leave events are direct — they do not bubble (WPF convention)
    if (event.type == MouseEventType::Enter || event.type == MouseEventType::Leave) {
        target->onMouseEvent(event);
        return;
    }
    
    // Set the original source so handlers can see which element was hit
    event.source = target;
    
    // Bubble up the visual tree: target → parent → grandparent → ... → root
    Element* current = target;
    while (current && !event.handled) {
        // Convert screen position to this element's local coordinates
        Rectf sBounds = current->screenBounds();
        event.position = {
            event.screenPosition.x - sBounds.x,
            event.screenPosition.y - sBounds.y
        };
        
        // If the handler returns true, treat it as handled (back-compat)
        if (current->onMouseEvent(event)) {
            event.handled = true;
        }
        
        current = current->parent();
    }
}

void InputManager::dispatchKeyEvent(Element* target, KeyEvent event) {
    if (!target) return;
    
    event.source = target;
    
    // Bubble up: focused element → parent → ... → root
    Element* current = target;
    while (current && !event.handled) {
        if (current->onKeyEvent(event)) {
            event.handled = true;
        }
        current = current->parent();
    }
}

} // namespace gut


// --- animation/Easing.cpp ---

#include <cmath>

namespace gut {

namespace easing {

// Pi constant
constexpr f32 PI = 3.14159265358979323846f;

f32 linear(f32 t) {
    return t;
}

// Quadratic
f32 easeInQuad(f32 t) {
    return t * t;
}

f32 easeOutQuad(f32 t) {
    return t * (2.0f - t);
}

f32 easeInOutQuad(f32 t) {
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

// Cubic
f32 easeInCubic(f32 t) {
    return t * t * t;
}

f32 easeOutCubic(f32 t) {
    f32 t1 = t - 1.0f;
    return t1 * t1 * t1 + 1.0f;
}

f32 easeInOutCubic(f32 t) {
    return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
}

// Quartic
f32 easeInQuart(f32 t) {
    return t * t * t * t;
}

f32 easeOutQuart(f32 t) {
    f32 t1 = t - 1.0f;
    return 1.0f - t1 * t1 * t1 * t1;
}

f32 easeInOutQuart(f32 t) {
    f32 t1 = t - 1.0f;
    return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - 8.0f * t1 * t1 * t1 * t1;
}

// Quintic
f32 easeInQuint(f32 t) {
    return t * t * t * t * t;
}

f32 easeOutQuint(f32 t) {
    f32 t1 = t - 1.0f;
    return 1.0f + t1 * t1 * t1 * t1 * t1;
}

f32 easeInOutQuint(f32 t) {
    f32 t1 = t - 1.0f;
    return t < 0.5f ? 16.0f * t * t * t * t * t : 1.0f + 16.0f * t1 * t1 * t1 * t1 * t1;
}

// Sine
f32 easeInSine(f32 t) {
    return 1.0f - std::cos(t * PI * 0.5f);
}

f32 easeOutSine(f32 t) {
    return std::sin(t * PI * 0.5f);
}

f32 easeInOutSine(f32 t) {
    return 0.5f * (1.0f - std::cos(PI * t));
}

// Exponential
f32 easeInExpo(f32 t) {
    return t == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * (t - 1.0f));
}

f32 easeOutExpo(f32 t) {
    return t == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
}

f32 easeInOutExpo(f32 t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    if (t < 0.5f) return 0.5f * std::pow(2.0f, 20.0f * t - 10.0f);
    return 1.0f - 0.5f * std::pow(2.0f, -20.0f * t + 10.0f);
}

// Circular
f32 easeInCirc(f32 t) {
    return 1.0f - std::sqrt(1.0f - t * t);
}

f32 easeOutCirc(f32 t) {
    f32 t1 = t - 1.0f;
    return std::sqrt(1.0f - t1 * t1);
}

f32 easeInOutCirc(f32 t) {
    if (t < 0.5f) {
        return 0.5f * (1.0f - std::sqrt(1.0f - 4.0f * t * t));
    }
    f32 t1 = 2.0f * t - 2.0f;
    return 0.5f * (std::sqrt(1.0f - t1 * t1) + 1.0f);
}

// Back (overshoot)
constexpr f32 c1 = 1.70158f;
constexpr f32 c2 = c1 * 1.525f;
constexpr f32 c3 = c1 + 1.0f;

f32 easeInBack(f32 t) {
    return c3 * t * t * t - c1 * t * t;
}

f32 easeOutBack(f32 t) {
    f32 t1 = t - 1.0f;
    return 1.0f + c3 * t1 * t1 * t1 + c1 * t1 * t1;
}

f32 easeInOutBack(f32 t) {
    if (t < 0.5f) {
        return (std::pow(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) * 0.5f;
    }
    f32 t1 = 2.0f * t - 2.0f;
    return (t1 * t1 * ((c2 + 1.0f) * t1 + c2) + 2.0f) * 0.5f;
}

// Elastic
f32 easeInElastic(f32 t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    return -std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * (2.0f * PI / 3.0f));
}

f32 easeOutElastic(f32 t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    return std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * (2.0f * PI / 3.0f)) + 1.0f;
}

f32 easeInOutElastic(f32 t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    constexpr f32 c5 = (2.0f * PI) / 4.5f;
    if (t < 0.5f) {
        return -0.5f * std::pow(2.0f, 20.0f * t - 10.0f) * std::sin((20.0f * t - 11.125f) * c5);
    }
    return std::pow(2.0f, -20.0f * t + 10.0f) * std::sin((20.0f * t - 11.125f) * c5) * 0.5f + 1.0f;
}

// Bounce
f32 easeOutBounce(f32 t) {
    constexpr f32 n1 = 7.5625f;
    constexpr f32 d1 = 2.75f;
    
    if (t < 1.0f / d1) {
        return n1 * t * t;
    } else if (t < 2.0f / d1) {
        t -= 1.5f / d1;
        return n1 * t * t + 0.75f;
    } else if (t < 2.5f / d1) {
        t -= 2.25f / d1;
        return n1 * t * t + 0.9375f;
    } else {
        t -= 2.625f / d1;
        return n1 * t * t + 0.984375f;
    }
}

f32 easeInBounce(f32 t) {
    return 1.0f - easeOutBounce(1.0f - t);
}

f32 easeInOutBounce(f32 t) {
    return t < 0.5f
        ? (1.0f - easeOutBounce(1.0f - 2.0f * t)) * 0.5f
        : (1.0f + easeOutBounce(2.0f * t - 1.0f)) * 0.5f;
}

} // namespace easing

// EasingRegistry implementation

EasingRegistry& EasingRegistry::instance() {
    static EasingRegistry registry;
    return registry;
}

EasingRegistry::EasingRegistry() {
    // Register standard functions
    m_functions["linear"] = easing::linear;
    
    m_functions["easeInQuad"] = easing::easeInQuad;
    m_functions["easeOutQuad"] = easing::easeOutQuad;
    m_functions["easeInOutQuad"] = easing::easeInOutQuad;
    
    m_functions["easeInCubic"] = easing::easeInCubic;
    m_functions["easeOutCubic"] = easing::easeOutCubic;
    m_functions["easeInOutCubic"] = easing::easeInOutCubic;
    
    m_functions["easeInQuart"] = easing::easeInQuart;
    m_functions["easeOutQuart"] = easing::easeOutQuart;
    m_functions["easeInOutQuart"] = easing::easeInOutQuart;
    
    m_functions["easeInQuint"] = easing::easeInQuint;
    m_functions["easeOutQuint"] = easing::easeOutQuint;
    m_functions["easeInOutQuint"] = easing::easeInOutQuint;
    
    m_functions["easeInSine"] = easing::easeInSine;
    m_functions["easeOutSine"] = easing::easeOutSine;
    m_functions["easeInOutSine"] = easing::easeInOutSine;
    
    m_functions["easeInExpo"] = easing::easeInExpo;
    m_functions["easeOutExpo"] = easing::easeOutExpo;
    m_functions["easeInOutExpo"] = easing::easeInOutExpo;
    
    m_functions["easeInCirc"] = easing::easeInCirc;
    m_functions["easeOutCirc"] = easing::easeOutCirc;
    m_functions["easeInOutCirc"] = easing::easeInOutCirc;
    
    m_functions["easeInBack"] = easing::easeInBack;
    m_functions["easeOutBack"] = easing::easeOutBack;
    m_functions["easeInOutBack"] = easing::easeInOutBack;
    
    m_functions["easeInElastic"] = easing::easeInElastic;
    m_functions["easeOutElastic"] = easing::easeOutElastic;
    m_functions["easeInOutElastic"] = easing::easeInOutElastic;
    
    m_functions["easeInBounce"] = easing::easeInBounce;
    m_functions["easeOutBounce"] = easing::easeOutBounce;
    m_functions["easeInOutBounce"] = easing::easeInOutBounce;
}

void EasingRegistry::registerFunction(const std::string& name, EasingFunction func) {
    m_functions[name] = std::move(func);
}

EasingFunction EasingRegistry::get(const std::string& name) const {
    auto it = m_functions.find(name);
    if (it != m_functions.end()) {
        return it->second;
    }
    return easing::linear;
}

bool EasingRegistry::has(const std::string& name) const {
    return m_functions.find(name) != m_functions.end();
}

// CubicBezierEasing implementation

CubicBezierEasing::CubicBezierEasing(f32 x1, f32 y1, f32 x2, f32 y2)
    : m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2) {}

f32 CubicBezierEasing::sampleCurveX(f32 t) const {
    // Bezier formula for X: ((1-t)^3 * 0) + (3*(1-t)^2 * t * x1) + (3*(1-t) * t^2 * x2) + (t^3 * 1)
    return ((1.0f - t) * 3.0f * (1.0f - t) * t * m_x1) + (3.0f * (1.0f - t) * t * t * m_x2) + (t * t * t);
}

f32 CubicBezierEasing::sampleCurveY(f32 t) const {
    return ((1.0f - t) * 3.0f * (1.0f - t) * t * m_y1) + (3.0f * (1.0f - t) * t * t * m_y2) + (t * t * t);
}

f32 CubicBezierEasing::sampleCurveDerivativeX(f32 t) const {
    return 3.0f * (1.0f - t) * (1.0f - t) * m_x1 + 6.0f * (1.0f - t) * t * (m_x2 - m_x1) + 3.0f * t * t * (1.0f - m_x2);
}

f32 CubicBezierEasing::solveCurveX(f32 x, f32 epsilon) const {
    // Newton-Raphson iteration
    f32 t = x;
    for (int i = 0; i < 8; i++) {
        f32 x2 = sampleCurveX(t) - x;
        if (std::abs(x2) < epsilon) {
            return t;
        }
        f32 d = sampleCurveDerivativeX(t);
        if (std::abs(d) < 1e-6f) {
            break;
        }
        t = t - x2 / d;
    }
    
    // Fallback to bisection
    f32 t0 = 0.0f;
    f32 t1 = 1.0f;
    t = x;
    
    while (t0 < t1) {
        f32 x2 = sampleCurveX(t);
        if (std::abs(x2 - x) < epsilon) {
            return t;
        }
        if (x > x2) {
            t0 = t;
        } else {
            t1 = t;
        }
        t = (t1 - t0) * 0.5f + t0;
    }
    
    return t;
}

f32 CubicBezierEasing::operator()(f32 t) const {
    if (t == 0.0f || t == 1.0f) {
        return t;
    }
    return sampleCurveY(solveCurveX(t, 1e-6f));
}

CubicBezierEasing CubicBezierEasing::ease() {
    return CubicBezierEasing(0.25f, 0.1f, 0.25f, 1.0f);
}

CubicBezierEasing CubicBezierEasing::easeIn() {
    return CubicBezierEasing(0.42f, 0.0f, 1.0f, 1.0f);
}

CubicBezierEasing CubicBezierEasing::easeOut() {
    return CubicBezierEasing(0.0f, 0.0f, 0.58f, 1.0f);
}

CubicBezierEasing CubicBezierEasing::easeInOut() {
    return CubicBezierEasing(0.42f, 0.0f, 0.58f, 1.0f);
}

// SpringEasing implementation

SpringEasing::SpringEasing(f32 mass, f32 stiffness, f32 damping)
    : m_mass(mass), m_stiffness(stiffness), m_damping(damping) {}

f32 SpringEasing::operator()(f32 t) const {
    // Damped harmonic oscillator
    f32 omega0 = std::sqrt(m_stiffness / m_mass);
    f32 zeta = m_damping / (2.0f * std::sqrt(m_stiffness * m_mass));
    
    if (zeta < 1.0f) {
        // Underdamped
        f32 omega = omega0 * std::sqrt(1.0f - zeta * zeta);
        return 1.0f - std::exp(-zeta * omega0 * t) * 
               (std::cos(omega * t) + (zeta * omega0 / omega) * std::sin(omega * t));
    } else if (zeta == 1.0f) {
        // Critically damped
        return 1.0f - std::exp(-omega0 * t) * (1.0f + omega0 * t);
    } else {
        // Overdamped
        f32 s1 = -omega0 * (zeta + std::sqrt(zeta * zeta - 1.0f));
        f32 s2 = -omega0 * (zeta - std::sqrt(zeta * zeta - 1.0f));
        return 1.0f - (s2 * std::exp(s1 * t) - s1 * std::exp(s2 * t)) / (s2 - s1);
    }
}

} // namespace gut


// --- animation/Animation.cpp ---


namespace gut {

Animation::Animation() = default;
Animation::~Animation() {
    stop();
}

f32 Animation::progress() const {
    f32 dur = duration();
    if (dur <= 0) return 1.0f;
    return std::clamp(m_currentTime / dur, 0.0f, 1.0f);
}

void Animation::begin() {
    if (m_state == AnimationState::Playing) {
        stop();
    }
    
    m_state = AnimationState::Playing;
    m_currentTime = 0.0f;
    m_totalElapsed = 0.0f;
    m_currentIteration = 0;
    m_isReversing = false;
    
    Timeline::global()->add(this);
    started.emit();
}

void Animation::pause() {
    if (m_state == AnimationState::Playing) {
        m_state = AnimationState::Paused;
    }
}

void Animation::resume() {
    if (m_state == AnimationState::Paused) {
        m_state = AnimationState::Playing;
    }
}

void Animation::stop() {
    if (m_state != AnimationState::Stopped) {
        m_state = AnimationState::Stopped;
        Timeline::global()->remove(this);
        
        if (fillBehavior() == FillBehavior::Stop) {
            // Reset to initial value - subclass should handle this
            applyValue(0.0f);
        }
    }
}

void Animation::seek(f32 timeMs) {
    m_currentTime = std::clamp(timeMs, 0.0f, duration());
    f32 t = progress();
    
    if (m_isReversing) {
        t = 1.0f - t;
    }
    
    applyValue(t);
}

void Animation::update(f32 deltaMs) {
    if (m_state != AnimationState::Playing) return;
    
    // Apply speed ratio
    deltaMs *= speedRatio();
    
    // Handle begin time delay
    if (m_totalElapsed < beginTime()) {
        m_totalElapsed += deltaMs;
        if (m_totalElapsed < beginTime()) {
            return;
        }
        // Consume the remaining delta after begin time
        deltaMs = m_totalElapsed - beginTime();
    }
    
    m_currentTime += deltaMs;
    
    f32 dur = duration();
    
    if (m_currentTime >= dur) {
        // Handle auto-reverse
        if (autoReverse() && !m_isReversing) {
            m_isReversing = true;
            m_currentTime = dur - (m_currentTime - dur);
        } else {
            m_currentIteration++;
            m_isReversing = false;
            
            // Check repeat behavior
            bool shouldRepeat = false;
            switch (m_repeatBehavior.type) {
                case RepeatBehavior::Type::Forever:
                    shouldRepeat = true;
                    break;
                case RepeatBehavior::Type::Count:
                    shouldRepeat = m_currentIteration < static_cast<i32>(m_repeatBehavior.count);
                    break;
                case RepeatBehavior::Type::Duration:
                    shouldRepeat = m_totalElapsed < m_repeatBehavior.durationMs;
                    break;
            }
            
            if (shouldRepeat) {
                m_currentTime = m_currentTime - dur;
            } else {
                // Animation complete
                m_currentTime = dur;
                applyValue(1.0f);
                m_state = AnimationState::Stopped;
                Timeline::global()->remove(this);
                completed.emit();
                return;
            }
        }
    }
    
    // Calculate normalized time and apply
    f32 t = std::clamp(m_currentTime / dur, 0.0f, 1.0f);
    if (m_isReversing) {
        t = 1.0f - t;
    }
    
    // Apply easing function if set
    if (m_easingFunction) {
        t = m_easingFunction(t);
    }
    
    applyValue(t);
    
    m_totalElapsed += deltaMs;
}

// Storyboard implementation

Storyboard::Storyboard() = default;
Storyboard::~Storyboard() {
    stop();
}

void Storyboard::addAnimation(Ref<Animation> animation) {
    m_animations.push_back(std::move(animation));
}

void Storyboard::removeAnimation(Animation* animation) {
    auto it = std::find_if(m_animations.begin(), m_animations.end(),
                           [animation](const auto& a) { return a.get() == animation; });
    if (it != m_animations.end()) {
        m_animations.erase(it);
    }
}

void Storyboard::clear() {
    stop();
    m_animations.clear();
}

void Storyboard::begin() {
    m_completedCount = 0;
    
    for (auto& anim : m_animations) {
        anim->completed.connect([this]() {
            m_completedCount++;
            if (m_completedCount >= static_cast<i32>(m_animations.size())) {
                completed.emit();
            }
        });
        anim->begin();
    }
    
    Timeline::global()->add(this);
}

void Storyboard::pause() {
    for (auto& anim : m_animations) {
        anim->pause();
    }
}

void Storyboard::resume() {
    for (auto& anim : m_animations) {
        anim->resume();
    }
}

void Storyboard::stop() {
    Timeline::global()->remove(this);
    
    for (auto& anim : m_animations) {
        anim->stop();
    }
}

void Storyboard::update(f32 deltaMs) {
    for (auto& anim : m_animations) {
        anim->update(deltaMs);
    }
}

} // namespace gut


// --- animation/Timeline.cpp ---

#include <algorithm>

namespace gut {

// Global timeline instance
static Timeline* s_globalTimeline = nullptr;

Timeline::Timeline() = default;

Timeline::~Timeline() {
    stopAll();
}

Timeline* Timeline::global() {
    if (!s_globalTimeline) {
        s_globalTimeline = new Timeline();
    }
    return s_globalTimeline;
}

void Timeline::add(Animation* animation) {
    if (!animation) return;
    
    auto it = std::find(m_animations.begin(), m_animations.end(), animation);
    if (it == m_animations.end()) {
        m_animations.push_back(animation);
    }
}

void Timeline::remove(Animation* animation) {
    if (!animation) return;
    
    if (m_updating) {
        // Defer removal during update
        m_pendingRemoveAnimations.push_back(animation);
    } else {
        auto it = std::find(m_animations.begin(), m_animations.end(), animation);
        if (it != m_animations.end()) {
            m_animations.erase(it);
        }
    }
}

void Timeline::add(Storyboard* storyboard) {
    if (!storyboard) return;
    
    auto it = std::find(m_storyboards.begin(), m_storyboards.end(), storyboard);
    if (it == m_storyboards.end()) {
        m_storyboards.push_back(storyboard);
    }
}

void Timeline::remove(Storyboard* storyboard) {
    if (!storyboard) return;
    
    if (m_updating) {
        m_pendingRemoveStoryboards.push_back(storyboard);
    } else {
        auto it = std::find(m_storyboards.begin(), m_storyboards.end(), storyboard);
        if (it != m_storyboards.end()) {
            m_storyboards.erase(it);
        }
    }
}

void Timeline::update(f32 deltaMs) {
    // Apply time scale
    deltaMs *= m_timeScale;
    
    m_currentTime += deltaMs;
    m_updating = true;
    
    // Update all animations
    for (auto* animation : m_animations) {
        animation->update(deltaMs);
    }
    
    // Update all storyboards
    for (auto* storyboard : m_storyboards) {
        storyboard->update(deltaMs);
    }
    
    m_updating = false;
    
    // Process pending removals
    for (auto* anim : m_pendingRemoveAnimations) {
        auto it = std::find(m_animations.begin(), m_animations.end(), anim);
        if (it != m_animations.end()) {
            m_animations.erase(it);
        }
    }
    m_pendingRemoveAnimations.clear();
    
    for (auto* sb : m_pendingRemoveStoryboards) {
        auto it = std::find(m_storyboards.begin(), m_storyboards.end(), sb);
        if (it != m_storyboards.end()) {
            m_storyboards.erase(it);
        }
    }
    m_pendingRemoveStoryboards.clear();
}

void Timeline::pauseAll() {
    for (auto* animation : m_animations) {
        animation->pause();
    }
}

void Timeline::resumeAll() {
    for (auto* animation : m_animations) {
        animation->resume();
    }
}

void Timeline::stopAll() {
    // Copy the vector as stopping may modify it
    auto animsCopy = m_animations;
    for (auto* animation : animsCopy) {
        animation->stop();
    }
    m_animations.clear();
    
    auto sbCopy = m_storyboards;
    for (auto* sb : sbCopy) {
        sb->stop();
    }
    m_storyboards.clear();
}

bool Timeline::hasActiveAnimations() const {
    return !m_animations.empty() || !m_storyboards.empty();
}

} // namespace gut


// --- style/Selector.cpp ---


namespace gut {

// Selector implementation is in Style.cpp

} // namespace gut


// --- style/Style.cpp ---

#include <sstream>
#include <cctype>

namespace gut {

// ============================================================================
// Selector
// ============================================================================

Selector Selector::parse(StringView selectorStr) {
    Selector selector;
    Segment currentSegment;
    
    String str(selectorStr);
    usize pos = 0;
    
    while (pos < str.length()) {
        // Skip whitespace
        while (pos < str.length() && std::isspace(str[pos])) {
            pos++;
        }
        
        if (pos >= str.length()) break;
        
        // Check for combinators
        if (!currentSegment.parts.empty()) {
            if (str[pos] == '>') {
                selector.m_segments.push_back(std::move(currentSegment));
                currentSegment = {};
                currentSegment.combinator = Combinator::Child;
                pos++;
                continue;
            } else if (str[pos] == '+') {
                selector.m_segments.push_back(std::move(currentSegment));
                currentSegment = {};
                currentSegment.combinator = Combinator::Adjacent;
                pos++;
                continue;
            } else if (str[pos] == '~') {
                selector.m_segments.push_back(std::move(currentSegment));
                currentSegment = {};
                currentSegment.combinator = Combinator::Sibling;
                pos++;
                continue;
            } else if (std::isspace(str[pos - 1])) {
                // Descendant combinator (whitespace)
                selector.m_segments.push_back(std::move(currentSegment));
                currentSegment = {};
                currentSegment.combinator = Combinator::Descendant;
            }
        }
        
        // Parse selector part
        if (str[pos] == '*') {
            currentSegment.parts.emplace_back(PartType::Universal, "*");
            pos++;
        } else if (str[pos] == '#') {
            pos++;
            String id;
            while (pos < str.length() && (std::isalnum(str[pos]) || str[pos] == '-' || str[pos] == '_')) {
                id += str[pos++];
            }
            currentSegment.parts.emplace_back(PartType::Id, std::move(id));
        } else if (str[pos] == '.') {
            pos++;
            String className;
            while (pos < str.length() && (std::isalnum(str[pos]) || str[pos] == '-' || str[pos] == '_')) {
                className += str[pos++];
            }
            currentSegment.parts.emplace_back(PartType::Class, std::move(className));
        } else if (str[pos] == ':') {
            pos++;
            String pseudoClass;
            while (pos < str.length() && (std::isalnum(str[pos]) || str[pos] == '-')) {
                pseudoClass += str[pos++];
            }
            currentSegment.parts.emplace_back(PartType::PseudoClass, std::move(pseudoClass));
        } else if (std::isalpha(str[pos])) {
            String typeName;
            while (pos < str.length() && (std::isalnum(str[pos]) || str[pos] == '-' || str[pos] == '_')) {
                typeName += str[pos++];
            }
            currentSegment.parts.emplace_back(PartType::Type, std::move(typeName));
        } else {
            pos++;  // Skip unknown character
        }
    }
    
    if (!currentSegment.parts.empty()) {
        selector.m_segments.push_back(std::move(currentSegment));
    }
    
    return selector;
}

bool Selector::matches(const Element& element) const {
    if (m_segments.empty()) return false;
    
    // Start from the last segment and work backwards
    return matchesSegment(element, m_segments.size() - 1);
}

bool Selector::matchesSegment(const Element& element, usize segmentIndex) const {
    const Segment& segment = m_segments[segmentIndex];
    
    // Check all parts in this segment
    for (const auto& part : segment.parts) {
        if (!matchesPart(element, part)) {
            return false;
        }
    }
    
    // If this is the first segment, we're done
    if (segmentIndex == 0) {
        return true;
    }
    
    // Check previous segments based on combinator
    switch (segment.combinator) {
        case Combinator::None:
        case Combinator::Descendant: {
            // Match any ancestor
            Element* ancestor = element.parent();
            while (ancestor) {
                if (matchesSegment(*ancestor, segmentIndex - 1)) {
                    return true;
                }
                ancestor = ancestor->parent();
            }
            return false;
        }
        
        case Combinator::Child: {
            // Match direct parent
            Element* parent = element.parent();
            return parent && matchesSegment(*parent, segmentIndex - 1);
        }
        
        case Combinator::Adjacent:
        case Combinator::Sibling:
            // TODO: Implement sibling matching
            return false;
    }
    
    return false;
}

bool Selector::matchesPart(const Element& element, const Part& part) const {
    switch (part.type) {
        case PartType::Universal:
            return true;
            
        case PartType::Type:
            return element.typeInfo().name() == part.value;
            
        case PartType::Id:
            return element.id() == part.value;
            
        case PartType::Class: {
            const String& classes = element.className();
            return classes == part.value ||
                   classes.find(part.value + " ") == 0 ||
                   classes.find(" " + part.value + " ") != String::npos ||
                   (classes.length() > part.value.length() &&
                    classes.rfind(" " + part.value) == classes.length() - part.value.length() - 1);
        }
            
        case PartType::PseudoClass:
            if (part.value == "hover") return element.isHovered();
            if (part.value == "pressed" || part.value == "active") return element.isPressed();
            if (part.value == "focus" || part.value == "focused") return element.isFocused();
            if (part.value == "disabled") return !element.isEnabled();
            if (part.value == "enabled") return element.isEnabled();
            return false;
    }
    
    return false;
}

std::tuple<u32, u32, u32> Selector::specificity() const {
    u32 ids = 0;
    u32 classes = 0;
    u32 types = 0;
    
    for (const auto& segment : m_segments) {
        for (const auto& part : segment.parts) {
            switch (part.type) {
                case PartType::Id:
                    ids++;
                    break;
                case PartType::Class:
                case PartType::PseudoClass:
                    classes++;
                    break;
                case PartType::Type:
                    types++;
                    break;
                case PartType::Universal:
                    break;
            }
        }
    }
    
    return {ids, classes, types};
}

bool Selector::moreSpecificThan(const Selector& other) const {
    auto [a1, b1, c1] = specificity();
    auto [a2, b2, c2] = other.specificity();
    
    if (a1 != a2) return a1 > a2;
    if (b1 != b2) return b1 > b2;
    return c1 > c2;
}

String Selector::toString() const {
    String result;
    
    for (usize i = 0; i < m_segments.size(); ++i) {
        const auto& segment = m_segments[i];
        
        if (i > 0) {
            switch (segment.combinator) {
                case Combinator::Descendant:
                    result += " ";
                    break;
                case Combinator::Child:
                    result += " > ";
                    break;
                case Combinator::Adjacent:
                    result += " + ";
                    break;
                case Combinator::Sibling:
                    result += " ~ ";
                    break;
                case Combinator::None:
                    break;
            }
        }
        
        for (const auto& part : segment.parts) {
            switch (part.type) {
                case PartType::Universal:
                    result += "*";
                    break;
                case PartType::Type:
                    result += part.value;
                    break;
                case PartType::Id:
                    result += "#" + part.value;
                    break;
                case PartType::Class:
                    result += "." + part.value;
                    break;
                case PartType::PseudoClass:
                    result += ":" + part.value;
                    break;
            }
        }
    }
    
    return result;
}

// ============================================================================
// StyleRule
// ============================================================================

StyleRule::StyleRule(Selector sel) : m_selector(std::move(sel)) {}

void StyleRule::setProperty(StringView name, StyleValue value) {
    m_properties[String(name)] = std::move(value);
}

std::optional<StyleValue> StyleRule::getProperty(StringView name) const {
    auto it = m_properties.find(String(name));
    if (it != m_properties.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool StyleRule::hasProperty(StringView name) const {
    return m_properties.contains(String(name));
}

// ============================================================================
// Style
// ============================================================================

void Style::set(StringView name, StyleValue value) {
    m_properties[String(name)] = std::move(value);
}

std::optional<StyleValue> Style::get(StringView name) const {
    auto it = m_properties.find(String(name));
    if (it != m_properties.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool Style::has(StringView name) const {
    return m_properties.contains(String(name));
}

void Style::remove(StringView name) {
    m_properties.erase(String(name));
}

void Style::clear() {
    m_properties.clear();
}

Style& Style::background(Color color) {
    set("background", color);
    return *this;
}

Style& Style::foreground(Color color) {
    set("foreground", color);
    return *this;
}

Style& Style::margin(Thickness value) {
    set("margin", value);
    return *this;
}

Style& Style::padding(Thickness value) {
    set("padding", value);
    return *this;
}

Style& Style::width(f32 value) {
    set("width", value);
    return *this;
}

Style& Style::height(f32 value) {
    set("height", value);
    return *this;
}

Style& Style::fontSize(f32 value) {
    set("font-size", value);
    return *this;
}

Style& Style::opacity(f32 value) {
    set("opacity", value);
    return *this;
}

Style& Style::visibility(Visibility value) {
    set("visibility", value);
    return *this;
}

} // namespace gut


// --- style/StyleSheet.cpp ---

#include <algorithm>
#include <sstream>

namespace gut {

Ref<StyleSheet> StyleSheet::parse(StringView css) {
    auto stylesheet = make<StyleSheet>();
    
    // Simple CSS parser
    String str(css);
    usize pos = 0;
    
    while (pos < str.length()) {
        // Skip whitespace and comments
        while (pos < str.length()) {
            if (std::isspace(str[pos])) {
                pos++;
            } else if (pos + 1 < str.length() && str[pos] == '/' && str[pos + 1] == '*') {
                // Block comment
                pos += 2;
                while (pos + 1 < str.length() && !(str[pos] == '*' && str[pos + 1] == '/')) {
                    pos++;
                }
                pos += 2;
            } else {
                break;
            }
        }
        
        if (pos >= str.length()) break;
        
        // Find selector (up to '{')
        usize selectorStart = pos;
        while (pos < str.length() && str[pos] != '{') {
            pos++;
        }
        
        if (pos >= str.length()) break;
        
        String selectorStr = str.substr(selectorStart, pos - selectorStart);
        pos++;  // Skip '{'
        
        // Find declarations (up to '}')
        usize declStart = pos;
        while (pos < str.length() && str[pos] != '}') {
            pos++;
        }
        
        String declarations = str.substr(declStart, pos - declStart);
        pos++;  // Skip '}'
        
        // Parse selector
        Selector selector = Selector::parse(selectorStr);
        if (!selector.isValid()) continue;
        
        StyleRule rule(std::move(selector));
        
        // Parse declarations
        std::istringstream declStream(declarations);
        String declaration;
        while (std::getline(declStream, declaration, ';')) {
            // Trim whitespace
            usize start = declaration.find_first_not_of(" \t\n\r");
            if (start == String::npos) continue;
            
            usize end = declaration.find_last_not_of(" \t\n\r");
            declaration = declaration.substr(start, end - start + 1);
            
            // Split into property and value
            usize colonPos = declaration.find(':');
            if (colonPos == String::npos) continue;
            
            String property = declaration.substr(0, colonPos);
            String value = declaration.substr(colonPos + 1);
            
            // Trim property and value
            property.erase(0, property.find_first_not_of(" \t"));
            property.erase(property.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            // Parse value based on property
            // TODO: More sophisticated value parsing
            StyleValue styleValue;
            
            // Try to parse as color
            if (value.length() >= 4 && value[0] == '#') {
                u32 hex = 0;
                std::istringstream hexStream(value.substr(1));
                hexStream >> std::hex >> hex;
                if (value.length() == 4) {
                    // #RGB -> #RRGGBB
                    u32 r = (hex >> 8) & 0xF;
                    u32 g = (hex >> 4) & 0xF;
                    u32 b = hex & 0xF;
                    hex = (r << 20) | (r << 16) | (g << 12) | (g << 8) | (b << 4) | b;
                }
                styleValue = Color::fromHex(hex);
            }
            // Try to parse as number
            else {
                try {
                    f32 num = std::stof(value);
                    styleValue = num;
                } catch (...) {
                    styleValue = value;  // Keep as string
                }
            }
            
            rule.setProperty(property, std::move(styleValue));
        }
        
        stylesheet->addRule(std::move(rule));
    }
    
    return stylesheet;
}

Ref<StyleSheet> StyleSheet::loadFromFile(StringView path) {
    // TODO: Implement file loading
    (void)path;
    return make<StyleSheet>();
}

void StyleSheet::addRule(StyleRule rule) {
    m_rules.push_back(std::move(rule));
}

void StyleSheet::clear() {
    m_rules.clear();
}

std::vector<const StyleRule*> StyleSheet::matchingRules(const Element& element) const {
    std::vector<const StyleRule*> result;
    
    for (const auto& rule : m_rules) {
        if (rule.selector().matches(element)) {
            result.push_back(&rule);
        }
    }
    
    // Sort by specificity (least specific first, so later rules override)
    std::sort(result.begin(), result.end(), [](const StyleRule* a, const StyleRule* b) {
        return b->selector().moreSpecificThan(a->selector());
    });
    
    return result;
}

Ref<Style> StyleSheet::computeStyle(const Element& element) const {
    auto style = make<Style>();
    
    auto rules = matchingRules(element);
    for (const auto* rule : rules) {
        for (const auto& [name, value] : rule->properties()) {
            style->set(name, value);
        }
    }
    
    return style;
}

// ============================================================================
// Theme
// ============================================================================

void Theme::addStyleSheet(Ref<StyleSheet> stylesheet) {
    m_styleSheets.push_back(std::move(stylesheet));
}

void Theme::setColor(StringView name, Color color) {
    m_colors[String(name)] = color;
}

std::optional<Color> Theme::getColor(StringView name) const {
    auto it = m_colors.find(String(name));
    if (it != m_colors.end()) {
        return it->second;
    }
    return std::nullopt;
}

Ref<Style> Theme::computeStyle(const Element& element) const {
    auto style = make<Style>();
    
    for (const auto& stylesheet : m_styleSheets) {
        auto rules = stylesheet->matchingRules(element);
        for (const auto* rule : rules) {
            for (const auto& [name, value] : rule->properties()) {
                style->set(name, value);
            }
        }
    }
    
    return style;
}

Ref<Theme> Theme::defaultLight() {
    auto theme = make<Theme>();
    
    // Define colors
    theme->setColor("background", Color::white());
    theme->setColor("foreground", Color::black());
    theme->setColor("primary", Color::fromHex(0x0078D4));
    theme->setColor("secondary", Color::fromHex(0x6C757D));
    theme->setColor("border", Color::fromHex(0xDEE2E6));
    
    // Create default stylesheet
    auto stylesheet = make<StyleSheet>();
    
    // Button styles
    StyleRule buttonRule(Selector::parse("Button"));
    buttonRule.setProperty("background", Color::fromHex(0xE9ECEF));
    buttonRule.setProperty("foreground", Color::black());
    buttonRule.setProperty("border-color", Color::fromHex(0xCED4DA));
    stylesheet->addRule(std::move(buttonRule));
    
    StyleRule buttonHoverRule(Selector::parse("Button:hover"));
    buttonHoverRule.setProperty("background", Color::fromHex(0xDEE2E6));
    stylesheet->addRule(std::move(buttonHoverRule));
    
    StyleRule buttonPressedRule(Selector::parse("Button:pressed"));
    buttonPressedRule.setProperty("background", Color::fromHex(0xCED4DA));
    stylesheet->addRule(std::move(buttonPressedRule));
    
    theme->addStyleSheet(std::move(stylesheet));
    
    return theme;
}

Ref<Theme> Theme::defaultDark() {
    auto theme = make<Theme>();
    
    // Define colors
    theme->setColor("background", Color::fromHex(0x1E1E1E));
    theme->setColor("foreground", Color::white());
    theme->setColor("primary", Color::fromHex(0x0078D4));
    theme->setColor("secondary", Color::fromHex(0x6C757D));
    theme->setColor("border", Color::fromHex(0x3C3C3C));
    
    // Create default stylesheet
    auto stylesheet = make<StyleSheet>();
    
    // Button styles
    StyleRule buttonRule(Selector::parse("Button"));
    buttonRule.setProperty("background", Color::fromHex(0x3C3C3C));
    buttonRule.setProperty("foreground", Color::white());
    buttonRule.setProperty("border-color", Color::fromHex(0x5C5C5C));
    stylesheet->addRule(std::move(buttonRule));
    
    StyleRule buttonHoverRule(Selector::parse("Button:hover"));
    buttonHoverRule.setProperty("background", Color::fromHex(0x4C4C4C));
    stylesheet->addRule(std::move(buttonHoverRule));
    
    StyleRule buttonPressedRule(Selector::parse("Button:pressed"));
    buttonPressedRule.setProperty("background", Color::fromHex(0x2C2C2C));
    stylesheet->addRule(std::move(buttonPressedRule));
    
    theme->addStyleSheet(std::move(stylesheet));
    
    return theme;
}

} // namespace gut


// --- resources/Font.cpp ---

#include <cstring>

namespace gut {

// FontFace implementation

FontFace::FontFace() = default;
FontFace::~FontFace() = default;

const Glyph* FontFace::glyph(u32 codepoint) const {
    auto it = m_glyphs.find(codepoint);
    if (it != m_glyphs.end()) {
        return &it->second;
    }
    
    // Return null glyph or replacement character
    it = m_glyphs.find(0xFFFD); // Unicode replacement character
    if (it != m_glyphs.end()) {
        return &it->second;
    }
    
    return nullptr;
}

f32 FontFace::kerning(u32 left, u32 right) const {
    u64 key = (static_cast<u64>(left) << 32) | right;
    auto it = m_kerning.find(key);
    if (it != m_kerning.end()) {
        return it->second;
    }
    return 0.0f;
}

f32 FontFace::measureWidth(const std::string& text) const {
    f32 width = 0;
    u32 prevCodepoint = 0;
    
    // Simple ASCII iteration (full implementation would use UTF-8 decoding)
    for (char c : text) {
        u32 codepoint = static_cast<u32>(static_cast<unsigned char>(c));
        
        const Glyph* g = glyph(codepoint);
        if (g) {
            // Add kerning
            if (prevCodepoint != 0) {
                width += kerning(prevCodepoint, codepoint);
            }
            width += g->advance;
        }
        
        prevCodepoint = codepoint;
    }
    
    return width;
}

Size2f FontFace::measureText(const std::string& text, f32 maxWidth) const {
    if (text.empty()) {
        return {0, m_lineHeight};
    }
    
    f32 currentLineWidth = 0;
    f32 maxLineWidth = 0;
    i32 lineCount = 1;
    u32 prevCodepoint = 0;
    
    // Simple ASCII iteration
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        
        // Handle newlines
        if (c == '\n') {
            maxLineWidth = std::max(maxLineWidth, currentLineWidth);
            currentLineWidth = 0;
            lineCount++;
            prevCodepoint = 0;
            continue;
        }
        
        u32 codepoint = static_cast<u32>(static_cast<unsigned char>(c));
        const Glyph* g = glyph(codepoint);
        
        if (g) {
            f32 advance = g->advance;
            if (prevCodepoint != 0) {
                advance += kerning(prevCodepoint, codepoint);
            }
            
            // Word wrap check
            if (maxWidth > 0 && currentLineWidth + advance > maxWidth && currentLineWidth > 0) {
                maxLineWidth = std::max(maxLineWidth, currentLineWidth);
                currentLineWidth = advance;
                lineCount++;
            } else {
                currentLineWidth += advance;
            }
        }
        
        prevCodepoint = codepoint;
    }
    
    maxLineWidth = std::max(maxLineWidth, currentLineWidth);
    
    return {maxLineWidth, static_cast<f32>(lineCount) * m_lineHeight};
}

// Font implementation using stb_truetype

// stb_truetype info stored inline (to avoid exposing stb headers in public API)
struct StbFontInfo {
    stbtt_fontinfo info;
    bool valid = false;
};

Font::Font() = default;

Font::~Font() {
    clearCache();
}

bool Font::loadFromMemory(const u8* data, size_t size) {
    if (!data || size == 0) {
        return false;
    }
    
    // Copy font data (stb_truetype requires data to persist)
    m_fontData.assign(data, data + size);
    
    return initStbFont();
}

bool Font::initStbFont() {
    if (m_fontData.empty()) {
        return false;
    }
    
    // Initialize stb_truetype
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, m_fontData.data(), 0)) {
        m_fontData.clear();
        return false;
    }
    
    m_stbInitialized = true;
    
    // ---- Extract font family name from the name table ----
    // Prefer nameID 16 (Typographic/Preferred Family) which gives the pure
    // family name without weight/style qualifiers (e.g. "Segoe UI" for all
    // variants).  Fall back to nameID 1 (Font Family) which may include
    // the style (e.g. "Segoe UI Bold").
    if (m_family.empty()) {
        auto extractName = [&](int nameID) -> std::string {
            int nameLen = 0;
            // Try platform 3 (Windows), encoding 1 (Unicode BMP), language 0x0409 (English US)
            const char* nameData = stbtt_GetFontNameString(&info, &nameLen,
                3, 1, 0x0409, nameID);
            if (nameData && nameLen > 0) {
                std::string fam;
                fam.reserve(nameLen / 2);
                for (int i = 0; i + 1 < nameLen; i += 2) {
                    unsigned char hi = static_cast<unsigned char>(nameData[i]);
                    unsigned char lo = static_cast<unsigned char>(nameData[i + 1]);
                    char16_t ch = static_cast<char16_t>((hi << 8) | lo);
                    if (ch < 128) fam.push_back(static_cast<char>(ch));
                    else          fam.push_back('?');
                }
                if (!fam.empty()) return fam;
            }
            // Fallback: platform 1 (Macintosh), encoding 0 (Roman), language 0 (English)
            nameData = stbtt_GetFontNameString(&info, &nameLen, 1, 0, 0, nameID);
            if (nameData && nameLen > 0)
                return std::string(nameData, nameLen);
            return {};
        };

        // nameID 16 = Typographic Family (preferred)
        m_family = extractName(16);
        // nameID 1 = Font Family (fallback)
        if (m_family.empty())
            m_family = extractName(1);
    }
    
    // ---- Extract weight from OS/2 table usWeightClass (offset 4) ----
    if (m_weight == FontWeight::Normal) {
        stbtt_uint8* data = m_fontData.data();
        stbtt_uint32 os2 = stbtt__find_table(data, info.fontstart, "OS/2");
        if (os2) {
            int wc = static_cast<int>(ttUSHORT(data + os2 + 4)); // usWeightClass
            // Map usWeightClass to FontWeight enum
            if      (wc <= 150) m_weight = FontWeight::Thin;
            else if (wc <= 250) m_weight = FontWeight::ExtraLight;
            else if (wc <= 350) m_weight = FontWeight::Light;
            else if (wc <= 450) m_weight = FontWeight::Normal;
            else if (wc <= 550) m_weight = FontWeight::Medium;
            else if (wc <= 650) m_weight = FontWeight::SemiBold;
            else if (wc <= 750) m_weight = FontWeight::Bold;
            else if (wc <= 850) m_weight = FontWeight::ExtraBold;
            else                m_weight = FontWeight::Black;
        }
    }
    
    // ---- Extract style from head table macStyle (offset 44) ----
    if (m_style == FontStyle::Normal && info.head) {
        stbtt_uint8* data = m_fontData.data();
        int macStyle = static_cast<int>(ttUSHORT(data + info.head + 44));
        if (macStyle & 2) // bit 1 = italic
            m_style = FontStyle::Italic;
    }
    
    return true;
}

bool Font::hasGlyph(u32 codepoint) const {
    if (!m_stbInitialized || m_fontData.empty()) {
        return codepoint >= 32 && codepoint < 127; // fallback
    }
    
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, m_fontData.data(), 0)) {
        return false;
    }
    
    return stbtt_FindGlyphIndex(&info, static_cast<int>(codepoint)) != 0;
}

Ref<FontFace> Font::getFace(f32 size, RenderBackend* backend) {
    // Use fixed-point key for cache (size * 10 for 0.1 precision)
    i32 sizeKey = static_cast<i32>(size * 10.0f);
    
    // Check cache
    auto it = m_faces.find(sizeKey);
    if (it != m_faces.end()) {
        return it->second;
    }
    
    auto face = makeRef<FontFace>();
    face->m_size = size;
    
    // If we don't have stb_truetype initialized, create placeholder glyphs
    if (!m_stbInitialized || m_fontData.empty()) {
        face->m_lineHeight = size * 1.2f;
        face->m_ascender = size * 0.8f;
        face->m_descender = -size * 0.2f;
        
        // Placeholder glyphs for ASCII
        for (u32 c = 32; c < 127; ++c) {
            Glyph g;
            g.codepoint = c;
            g.advance = size * 0.6f;
            g.bearingX = 0;
            g.bearingY = size * 0.8f;
            g.width = size * 0.5f;
            g.height = size;
            g.u0 = 0; g.v0 = 0;
            g.u1 = 0; g.v1 = 0;
            face->m_glyphs[c] = g;
        }
        
        m_faces[sizeKey] = face;
        return face;
    }
    
    // Initialize stb_truetype for this rasterization
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, m_fontData.data(), 0)) {
        m_faces[sizeKey] = face;
        return face;
    }
    
    // Calculate scale for desired pixel size
    f32 scale = stbtt_ScaleForPixelHeight(&info, size);
    
    // Get font metrics
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
    
    face->m_ascender = ascent * scale;
    face->m_descender = descent * scale;
    face->m_lineHeight = (ascent - descent + lineGap) * scale;
    
    // Determine atlas size based on font size
    // Larger fonts need bigger atlases
    int atlasSize = 256;
    if (size >= 32) atlasSize = 512;
    if (size >= 64) atlasSize = 1024;
    if (size >= 128) atlasSize = 2048;
    
    face->m_atlasWidth = atlasSize;
    face->m_atlasHeight = atlasSize;
    face->m_atlasBitmap.resize(atlasSize * atlasSize, 0);
    
    // Bake ASCII characters (32-126) into atlas using stb_truetype
    // We use stbtt_BakeFontBitmap for simplicity
    const int firstChar = 32;
    const int numChars = 95; // 32-126 inclusive
    
    std::vector<stbtt_bakedchar> bakedChars(numChars);
    
    int result = stbtt_BakeFontBitmap(
        m_fontData.data(),
        0,                          // font index
        size,                       // pixel height
        face->m_atlasBitmap.data(), // output bitmap
        atlasSize,                  // bitmap width
        atlasSize,                  // bitmap height
        firstChar,                  // first char
        numChars,                   // num chars
        bakedChars.data()           // output char data
    );
    
    // If result is negative, not all chars fit - but we continue with what we have
    (void)result;
    
    // Convert baked chars to our Glyph format
    for (int i = 0; i < numChars; ++i) {
        const stbtt_bakedchar& bc = bakedChars[i];
        u32 codepoint = firstChar + i;
        
        Glyph g;
        g.codepoint = codepoint;
        g.advance = bc.xadvance;
        g.bearingX = bc.xoff;
        g.bearingY = -bc.yoff; // stb uses opposite sign convention
        g.width = static_cast<f32>(bc.x1 - bc.x0);
        g.height = static_cast<f32>(bc.y1 - bc.y0);
        
        // UV coordinates (normalized 0-1)
        g.u0 = bc.x0 / static_cast<f32>(atlasSize);
        g.v0 = bc.y0 / static_cast<f32>(atlasSize);
        g.u1 = bc.x1 / static_cast<f32>(atlasSize);
        g.v1 = bc.y1 / static_cast<f32>(atlasSize);
        
        face->m_glyphs[codepoint] = g;
    }
    
    // Build kerning table for common pairs
    for (int i = 0; i < numChars; ++i) {
        for (int j = 0; j < numChars; ++j) {
            int leftCodepoint = firstChar + i;
            int rightCodepoint = firstChar + j;
            
            int kern = stbtt_GetCodepointKernAdvance(&info, leftCodepoint, rightCodepoint);
            if (kern != 0) {
                u64 key = (static_cast<u64>(leftCodepoint) << 32) | static_cast<u64>(rightCodepoint);
                face->m_kerning[key] = kern * scale;
            }
        }
    }
    
    face->m_needsUpload = true;
    m_faces[sizeKey] = face;
    return face;
}

void Font::preload(const std::vector<f32>& sizes, RenderBackend* backend) {
    for (f32 size : sizes) {
        getFace(size, backend);
    }
}

void Font::clearCache() {
    m_faces.clear();
}

// TextShaper implementation

TextShaper::TextShaper() = default;
TextShaper::~TextShaper() = default;

std::vector<ShapedGlyph> TextShaper::shape(FontFace* face, const std::string& text) const {
    std::vector<ShapedGlyph> result;
    if (!face || text.empty()) {
        return result;
    }
    
    f32 x = 0;
    f32 y = 0;
    u32 prevCodepoint = 0;
    
    for (char c : text) {
        u32 codepoint = static_cast<u32>(static_cast<unsigned char>(c));
        const Glyph* g = face->glyph(codepoint);
        
        if (g) {
            // Apply kerning
            if (prevCodepoint != 0) {
                x += face->kerning(prevCodepoint, codepoint);
            }
            
            ShapedGlyph shaped;
            shaped.glyph = g;
            shaped.x = x + g->bearingX;
            shaped.y = y;
            result.push_back(shaped);
            
            x += g->advance;
        }
        
        prevCodepoint = codepoint;
    }
    
    return result;
}

std::vector<std::string> TextShaper::breakWords(const std::string& text) const {
    std::vector<std::string> words;
    std::string currentWord;
    
    for (char c : text) {
        if (c == ' ' || c == '\t' || c == '\n') {
            if (!currentWord.empty()) {
                words.push_back(currentWord);
                currentWord.clear();
            }
            // Add whitespace as separate "word" to preserve spacing
            if (c == '\n') {
                words.push_back("\n");
            } else {
                words.push_back(std::string(1, c));
            }
        } else {
            currentWord += c;
        }
    }
    
    if (!currentWord.empty()) {
        words.push_back(currentWord);
    }
    
    return words;
}

std::vector<TextShaper::ShapedLine> TextShaper::shapeMultiline(
    FontFace* face, 
    const std::string& text,
    f32 maxWidth, 
    TextAlignment alignment) const 
{
    std::vector<ShapedLine> lines;
    if (!face || text.empty()) {
        return lines;
    }
    
    std::vector<std::string> words = breakWords(text);
    ShapedLine currentLine;
    f32 currentX = 0;
    
    for (const auto& word : words) {
        // Handle explicit newlines
        if (word == "\n") {
            lines.push_back(std::move(currentLine));
            currentLine = ShapedLine{};
            currentX = 0;
            continue;
        }
        
        f32 wordWidth = face->measureWidth(word);
        
        // Word wrap
        if (maxWidth > 0 && currentX + wordWidth > maxWidth && !currentLine.glyphs.empty()) {
            lines.push_back(std::move(currentLine));
            currentLine = ShapedLine{};
            currentX = 0;
            
            // Skip leading spaces on new line
            if (word == " " || word == "\t") {
                continue;
            }
        }
        
        // Shape the word
        u32 prevCodepoint = 0;
        for (char c : word) {
            u32 codepoint = static_cast<u32>(static_cast<unsigned char>(c));
            const Glyph* g = face->glyph(codepoint);
            
            if (g) {
                if (prevCodepoint != 0) {
                    currentX += face->kerning(prevCodepoint, codepoint);
                }
                
                ShapedGlyph shaped;
                shaped.glyph = g;
                shaped.x = currentX + g->bearingX;
                shaped.y = 0;
                currentLine.glyphs.push_back(shaped);
                
                currentX += g->advance;
            }
            
            prevCodepoint = codepoint;
        }
        
        currentLine.width = currentX;
    }
    
    // Add last line
    if (!currentLine.glyphs.empty()) {
        lines.push_back(std::move(currentLine));
    }
    
    // Apply text alignment
    if (maxWidth > 0 && alignment != TextAlignment::Left) {
        for (auto& line : lines) {
            f32 offset = 0;
            
            switch (alignment) {
                case TextAlignment::Center:
                    offset = (maxWidth - line.width) * 0.5f;
                    break;
                case TextAlignment::Right:
                    offset = maxWidth - line.width;
                    break;
                case TextAlignment::Justify:
                    // Justify would require adjusting space widths
                    break;
                default:
                    break;
            }
            
            if (offset > 0) {
                for (auto& glyph : line.glyphs) {
                    glyph.x += offset;
                }
            }
        }
    }
    
    return lines;
}

} // namespace gut


// --- resources/Texture.cpp ---


// Texture is an abstract base class.
// Concrete implementations are provided by the render backend.
// The constructor, destructor, and inline methods are defined in the header.


// --- resources/ResourceManager.cpp ---


namespace gut {

// Global instance
static ResourceManager* s_globalResourceManager = nullptr;

Resource::Resource() = default;
Resource::~Resource() = default;

ResourceManager::ResourceManager() = default;

ResourceManager::~ResourceManager() {
    unloadAll();
}

ResourceManager* ResourceManager::global() {
    if (!s_globalResourceManager) {
        s_globalResourceManager = new ResourceManager();
    }
    return s_globalResourceManager;
}

void ResourceManager::registerLoader(const std::string& type, std::unique_ptr<IResourceLoader> loader) {
    m_loaders[type] = std::move(loader);
}

void ResourceManager::setBasePath(const std::string& path) {
    m_basePath = path;
    // Ensure trailing slash
    if (!m_basePath.empty() && m_basePath.back() != '/' && m_basePath.back() != '\\') {
        m_basePath += '/';
    }
}

std::string ResourceManager::resolvePath(const std::string& path) const {
    // If path is absolute, use it directly
    if (!path.empty() && (path[0] == '/' || (path.length() > 1 && path[1] == ':'))) {
        return path;
    }
    return m_basePath + path;
}

void ResourceManager::cacheResource(const std::string& id, Ref<Resource> resource) {
    resource->setId(id);
    m_resources[id] = std::move(resource);
}

Ref<Texture> ResourceManager::loadTexture(const std::string& path) {
    // Check cache first
    auto it = m_resources.find(path);
    if (it != m_resources.end()) {
        // Note: This is a simplified cast - actual impl would verify type
        return Ref<Texture>();  // Return null, no concrete Texture implementation
    }
    
    std::string fullPath = resolvePath(path);
    
    // Try custom loader first
    auto loaderIt = m_loaders.find("texture");
    if (loaderIt != m_loaders.end()) {
        auto resource = loaderIt->second->load(fullPath);
        if (resource) {
            cacheResource(path, resource);
            // Would need to cast to Texture
        }
    }
    
    // No concrete texture implementation - return null
    return nullptr;
}

Ref<Texture> ResourceManager::loadTextureFromMemory(const void* data, size_t size, const std::string& id) {
    // Try custom loader first
    auto loaderIt = m_loaders.find("texture");
    if (loaderIt != m_loaders.end()) {
        auto resource = loaderIt->second->loadFromMemory(data, size, "texture");
        if (resource && !id.empty()) {
            cacheResource(id, resource);
        }
    }
    
    return nullptr;
}

Ref<Font> ResourceManager::loadFont(const std::string& path) {
    // Check cache first
    auto it = m_resources.find(path);
    if (it != m_resources.end()) {
        auto* font = dynamic_cast<Font*>(it->second.get());
        if (font) {
            font->addRef();
            return Ref<Font>(font);
        }
    }
    
    std::string fullPath = resolvePath(path);
    
    // Try custom loader first
    auto loaderIt = m_loaders.find("font");
    if (loaderIt != m_loaders.end()) {
        auto resource = loaderIt->second->load(fullPath);
        if (resource) {
            auto* font = dynamic_cast<Font*>(resource.get());
            if (font) {
                cacheResource(path, resource);
                font->addRef();
                return Ref<Font>(font);
            }
        }
    }
    
    // Create empty font placeholder
    auto font = gut::makeRef<Font>();
    font->m_status = ResourceStatus::NotLoaded;
    font->setId(path);
    cacheResource(path, font);
    
    return font;
}

Ref<Font> ResourceManager::loadFontFromMemory(const void* data, size_t size, const std::string& id) {
    // Try custom loader first
    auto loaderIt = m_loaders.find("font");
    if (loaderIt != m_loaders.end()) {
        auto resource = loaderIt->second->loadFromMemory(data, size, "font");
        if (resource) {
            auto* font = dynamic_cast<Font*>(resource.get());
            if (font) {
                if (!id.empty()) {
                    cacheResource(id, resource);
                }
                font->addRef();
                return Ref<Font>(font);
            }
        }
    }
    
    // Create empty font placeholder
    auto font = gut::makeRef<Font>();
    font->m_status = ResourceStatus::NotLoaded;
    if (!id.empty()) {
        cacheResource(id, font);
    }
    
    return font;
}

bool ResourceManager::has(const std::string& id) const {
    return m_resources.find(id) != m_resources.end();
}

void ResourceManager::unload(const std::string& id) {
    m_resources.erase(id);
}

void ResourceManager::unloadAll() {
    m_resources.clear();
}

ResourceManager::MemoryStats ResourceManager::memoryStats() const {
    MemoryStats stats;
    stats.totalResources = m_resources.size();
    
    for (const auto& [id, resource] : m_resources) {
        if (auto* font = dynamic_cast<Font*>(resource.get())) {
            stats.fontMemory += font->m_fontData.size();
        }
        // Texture memory estimation would require concrete implementation
    }
    
    return stats;
}

} // namespace gut


// --- layout/Constraints.cpp ---


namespace gut {

// Placeholder for layout constraints implementation

} // namespace gut


// --- layout/LayoutEngine.cpp ---


namespace gut {

// Layout is currently handled directly by Element::measure/arrange
// This file is a placeholder for future layout optimizations

} // namespace gut


// --- elements/Element.cpp ---

#include <cmath>
#include <algorithm>

namespace gut {

Element::Element() = default;

Element::~Element() = default;

Rectf Element::screenBounds() const {
    Rectf result = m_bounds;
    const Element* current = m_parent;
    while (current) {
        result.x += current->m_bounds.x;
        result.y += current->m_bounds.y;
        current = current->m_parent;
    }
    return result;
}

Element* Element::findById(StringView searchId) {
    if (id() == searchId) {
        return this;
    }
    
    for (usize i = 0; i < childCount(); ++i) {
        if (auto* child = childAt(i)) {
            if (auto* found = child->findById(searchId)) {
                return found;
            }
        }
    }
    
    return nullptr;
}

std::vector<Element*> Element::findByClass(StringView searchClass) {
    std::vector<Element*> result;
    
    // Check if this element's className contains the search class
    // Simple implementation: exact match or space-separated
    const String& classes = className();
    if (classes == searchClass || 
        classes.find(String(searchClass) + " ") != String::npos ||
        classes.find(" " + String(searchClass)) != String::npos) {
        result.push_back(this);
    }
    
    for (usize i = 0; i < childCount(); ++i) {
        if (auto* child = childAt(i)) {
            auto childResults = child->findByClass(searchClass);
            result.insert(result.end(), childResults.begin(), childResults.end());
        }
    }
    
    return result;
}

Size2f Element::measure(Size2f availableSize) {
    if (visibility() == Visibility::Collapsed) {
        m_desiredSize = {0, 0};
        return m_desiredSize;
    }
    
    // Apply margin
    const auto& m = margin();
    f32 availWidth = availableSize.width - m.horizontalSum();
    f32 availHeight = availableSize.height - m.verticalSum();
    
    // Apply explicit size constraints
    if (!std::isnan(width())) {
        availWidth = std::min(availWidth, width());
    }
    if (!std::isnan(height())) {
        availHeight = std::min(availHeight, height());
    }
    
    // Apply min/max constraints
    availWidth = std::clamp(availWidth, minWidth(), maxWidth());
    availHeight = std::clamp(availHeight, minHeight(), maxHeight());
    
    // Call subclass implementation
    Size2f desiredContent = measureOverride({availWidth, availHeight});
    
    // Apply explicit size
    if (!std::isnan(width())) {
        desiredContent.width = width();
    }
    if (!std::isnan(height())) {
        desiredContent.height = height();
    }
    
    // Apply min/max constraints to result
    desiredContent.width = std::clamp(desiredContent.width, minWidth(), maxWidth());
    desiredContent.height = std::clamp(desiredContent.height, minHeight(), maxHeight());
    
    // Add margin back
    m_desiredSize = {
        desiredContent.width + m.horizontalSum(),
        desiredContent.height + m.verticalSum()
    };
    
    return m_desiredSize;
}

void Element::arrange(Rectf finalRect) {
    if (visibility() == Visibility::Collapsed) {
        m_bounds = {finalRect.x, finalRect.y, 0, 0};
        return;
    }
    
    // Apply margin
    const auto& m = margin();
    Rectf contentRect = {
        finalRect.x + m.left,
        finalRect.y + m.top,
        finalRect.width - m.horizontalSum(),
        finalRect.height - m.verticalSum()
    };
    
    // Compute final size based on alignment
    f32 finalWidth = contentRect.width;
    f32 finalHeight = contentRect.height;
    
    if (horizontalAlignment() != HorizontalAlignment::Stretch) {
        finalWidth = std::min(m_desiredSize.width - m.horizontalSum(), contentRect.width);
    }
    if (verticalAlignment() != VerticalAlignment::Stretch) {
        finalHeight = std::min(m_desiredSize.height - m.verticalSum(), contentRect.height);
    }
    
    // Apply min/max constraints
    finalWidth = std::clamp(finalWidth, minWidth(), maxWidth());
    finalHeight = std::clamp(finalHeight, minHeight(), maxHeight());
    
    // Compute position based on alignment
    f32 x = contentRect.x;
    f32 y = contentRect.y;
    
    switch (horizontalAlignment()) {
        case HorizontalAlignment::Center:
            x += (contentRect.width - finalWidth) / 2;
            break;
        case HorizontalAlignment::Right:
            x += contentRect.width - finalWidth;
            break;
        default:
            break;
    }
    
    switch (verticalAlignment()) {
        case VerticalAlignment::Center:
            y += (contentRect.height - finalHeight) / 2;
            break;
        case VerticalAlignment::Bottom:
            y += contentRect.height - finalHeight;
            break;
        default:
            break;
    }
    
    // Call subclass implementation
    Size2f actualSize = arrangeOverride({finalWidth, finalHeight});
    
    m_bounds = {x, y, actualSize.width, actualSize.height};
    m_layoutDirty = false;
}

void Element::invalidateLayout() {
    m_layoutDirty = true;
    if (m_parent) {
        m_parent->invalidateLayout();
    }
}

void Element::invalidateRender() {
    m_renderDirty = true;
}

void Element::render(RenderContext& ctx) {
    if (visibility() != Visibility::Visible) {
        return;
    }
    
    ctx.save();
    ctx.translate(m_bounds.x, m_bounds.y);
    ctx.setOpacity(ctx.opacity() * opacity());

    // Apply per-element transforms around element centre
    f32 sx = scaleX(), sy = scaleY();
    f32 rot = rotation();
    f32 skx = skewX(), sky = skewY();
    bool hasTransform = (sx != 1.0f || sy != 1.0f ||
                         rot != 0.0f || skx != 0.0f || sky != 0.0f);
    if (hasTransform) {
        f32 cx = m_bounds.width * 0.5f;
        f32 cy = m_bounds.height * 0.5f;
        ctx.translate(cx, cy);
        if (rot != 0.0f) ctx.rotate(rot);
        if (sx != 1.0f || sy != 1.0f) ctx.scale(sx, sy);
        if (skx != 0.0f || sky != 0.0f) ctx.skew(skx, sky);
        ctx.translate(-cx, -cy);
    }
    
    // Only clip to bounds when explicitly requested
    if (clipToBounds()) {
        ctx.pushClip({0, 0, m_bounds.width, m_bounds.height}, getClipCornerRadius());
    }
    
    onRender(ctx);
    
    if (clipToBounds()) {
        ctx.popClip();
    }
    ctx.restore();
    
    m_renderDirty = false;
}

Element* Element::hitTest(Point2f point) {
    if (visibility() != Visibility::Visible || !isHitTestVisible()) {
        return nullptr;
    }
    
    // Account for per-element transforms (centred)
    f32 sx = scaleX(), sy = scaleY();
    f32 rot = rotation();
    f32 skx = skewX(), sky = skewY();
    bool hasTransform = (sx != 1.0f || sy != 1.0f ||
                         rot != 0.0f || skx != 0.0f || sky != 0.0f);
    if (hasTransform) {
        f32 cx = m_bounds.width * 0.5f;
        f32 cy = m_bounds.height * 0.5f;

        // Build the forward 2x2 matrix: R(rot) * S(sx,sy) * Sk(skx,sky)
        f32 cs = std::cos(rot), sn = std::sin(rot);
        f32 tx = std::tan(skx), ty = std::tan(sky);
        // R * S = [cs*sx  -sn*sy; sn*sx  cs*sy]
        // (R*S) * Sk = [cs*sx - sn*sy*ty    cs*sx*tx - sn*sy;
        //               sn*sx + cs*sy*ty    sn*sx*tx + cs*sy]
        f32 a = cs * sx - sn * sy * ty;
        f32 b = cs * sx * tx - sn * sy;
        f32 c = sn * sx + cs * sy * ty;
        f32 d = sn * sx * tx + cs * sy;

        // Inverse of 2x2 [a b; c d]
        f32 det = a * d - b * c;
        if (std::abs(det) > 1e-6f) {
            f32 inv = 1.0f / det;
            f32 px = point.x - cx;
            f32 py = point.y - cy;
            point.x = (d * px - b * py) * inv + cx;
            point.y = (-c * px + a * py) * inv + cy;
        }
    }
    
    if (!containsPoint(point)) {
        return nullptr;
    }
    
    // Check children in reverse order (top to bottom)
    for (isize i = static_cast<isize>(childCount()) - 1; i >= 0; --i) {
        if (auto* child = childAt(static_cast<usize>(i))) {
            Point2f localPoint = {
                point.x - child->m_bounds.x,
                point.y - child->m_bounds.y
            };
            if (auto* hit = child->hitTest(localPoint)) {
                return hit;
            }
        }
    }
    
    return this;
}

bool Element::containsPoint(Point2f point) const {
    return point.x >= 0 && point.x < m_bounds.width &&
           point.y >= 0 && point.y < m_bounds.height;
}

bool Element::onMouseEvent(const MouseEvent& event) {
    (void)event;
    return false;
}

bool Element::onKeyEvent(const KeyEvent& event) {
    (void)event;
    return false;
}

void Element::focus() {
    // This will be handled by FocusManager
    // For now, just set the flag
    setisFocused(true);
    onFocusGained();
    m_focusGained.emit();
}

void Element::unfocus() {
    setisFocused(false);
    onFocusLost();
    m_focusLost.emit();
}

Size2f Element::measureOverride(Size2f availableSize) {
    // Base implementation returns zero size
    (void)availableSize;
    return {0, 0};
}

Size2f Element::arrangeOverride(Size2f finalSize) {
    // Base implementation just returns the given size
    return finalSize;
}

void Element::onRender(RenderContext& ctx) {
    // Base implementation does nothing
    (void)ctx;
}

void Element::onFocusGained() {
    // Base implementation does nothing
}

void Element::onFocusLost() {
    // Base implementation does nothing
}

void Element::onMouseEnter() {
    setisHovered(true);
    m_mouseEntered.emit();
}

void Element::onMouseLeave() {
    setisHovered(false);
    m_mouseLeft.emit();
}

void Element::setParent(Element* parent) {
    m_parent = parent;
}

} // namespace gut


// --- elements/Panel.cpp ---

#include <algorithm>

namespace gut {

Panel::Panel(std::initializer_list<Ref<Element>> children) {
    for (auto& child : children) {
        addChild(child);
    }
}

Panel::~Panel() {
    clearChildren();
}

void Panel::addChild(Ref<Element> child) {
    if (!child) return;
    
    child->setParent(this);
    // Propagate context from parent
    if (context()) {
        child->setContext(context());
    }
    m_children.push_back(std::move(child));
    invalidateLayout();
}

void Panel::insertChild(usize index, Ref<Element> child) {
    if (!child) return;
    
    index = std::min(index, m_children.size());
    child->setParent(this);
    if (context()) {
        child->setContext(context());
    }
    m_children.insert(m_children.begin() + static_cast<ptrdiff_t>(index), std::move(child));
    invalidateLayout();
}

bool Panel::removeChild(Element* child) {
    auto it = std::find_if(m_children.begin(), m_children.end(),
        [child](const Ref<Element>& e) { return e.get() == child; });
    
    if (it != m_children.end()) {
        (*it)->setParent(nullptr);
        m_children.erase(it);
        invalidateLayout();
        return true;
    }
    return false;
}

void Panel::removeChildAt(usize index) {
    if (index < m_children.size()) {
        m_children[index]->setParent(nullptr);
        m_children.erase(m_children.begin() + static_cast<ptrdiff_t>(index));
        invalidateLayout();
    }
}

void Panel::clearChildren() {
    for (auto& child : m_children) {
        child->setParent(nullptr);
    }
    m_children.clear();
    invalidateLayout();
}

Element* Panel::childAt(usize index) const {
    return index < m_children.size() ? m_children[index].get() : nullptr;
}

Element* Panel::hitTest(Point2f point) {
    if (visibility() != Visibility::Visible || !isHitTestVisible()) {
        return nullptr;
    }
    
    if (!containsPoint(point)) {
        return nullptr;
    }
    
    // Build in reverse insertion order then sort descending by zIndex.
    // stable_sort preserves reverse-insertion order for equal zIndex,
    // so last-added children (visually topmost) are hit-tested first.
    std::vector<Element*> sorted;
    sorted.reserve(m_children.size());
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        sorted.push_back(it->get());
    }
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const Element* a, const Element* b) {
                         return a->zIndex() > b->zIndex();
                     });
    for (auto* child : sorted) {
        Point2f localPoint = {
            point.x - child->bounds().x,
            point.y - child->bounds().y
        };
        if (auto* hit = child->hitTest(localPoint)) {
            return hit;
        }
    }
    
    return this;
}

Size2f Panel::measureOverride(Size2f availableSize) {
    // Basic Panel layout: measure each child with available space,
    // return the max child desired size (overlay / fill behavior).
    f32 maxW = 0, maxH = 0;
    for (auto& child : m_children) {
        child->measure(availableSize);
        maxW = std::max(maxW, child->desiredSize().width);
        maxH = std::max(maxH, child->desiredSize().height);
    }
    return {maxW, maxH};
}

Size2f Panel::arrangeOverride(Size2f finalSize) {
    // Basic Panel layout: arrange each child filling the panel.
    for (auto& child : m_children) {
        child->arrange({0, 0, finalSize.width, finalSize.height});
    }
    return finalSize;
}

void Panel::onRender(RenderContext& ctx) {
    Rectf rect = {0, 0, bounds().width, bounds().height};
    f32 cr = cornerRadius();
    
    // Draw backdrop blur (frosted glass) if enabled
    if (backdropBlur() > 0) {
        ctx.drawBackdropBlur(rect, cr, backdropBlur(), backdropTint());
    }
    
    // Choose background color: pressed > hover > normal
    Color bg = background();
    if (isPressed() && pressedBackground().a > 0) {
        bg = pressedBackground();
    } else if (isHovered() && hoverBackground().a > 0) {
        bg = hoverBackground();
    }
    
    // Check for gradient background
    bool hasGradient = backgroundGradientTop().a > 0 && backgroundGradientBottom().a > 0;
    
    // Draw background — backgroundBrush takes precedence when not hovered/pressed
    if (m_backgroundBrush && !isPressed() && !isHovered()) {
        if (cr > 0) {
            ctx.fillRoundedRect(rect, cr, *m_backgroundBrush);
        } else {
            ctx.fillRect(rect, *m_backgroundBrush);
        }
    } else if (hasGradient && !isPressed() && !isHovered()) {
        // Use gradient (gradient takes precedence over solid bg when not hovered/pressed)
        if (cr > 0) {
            ctx.fillRoundedRectGradient(rect, cr, backgroundGradientTop(), backgroundGradientBottom());
        } else {
            ctx.fillRectGradient(rect, backgroundGradientTop(), backgroundGradientBottom());
        }
    } else if (bg.a > 0) {
        if (cr > 0) {
            ctx.fillRoundedRect(rect, cr, bg);
        } else {
            ctx.fillRect(rect, bg);
        }
    }
    
    // Draw inset shadow (inner shadow)
    if (insetShadowColor().a > 0 &&
        (insetShadowBlurRadius() > 0 || insetShadowOffsetX() != 0 || insetShadowOffsetY() != 0)) {
        ctx.drawInsetShadow(rect, cr, insetShadowColor(), insetShadowBlurRadius(),
                            insetShadowOffsetX(), insetShadowOffsetY());
    }
    
    // Draw border
    if (borderWidth() > 0 && borderColor().a > 0) {
        if (cr > 0) {
            ctx.strokeRoundedRect(rect, cr, borderColor(), borderWidth());
        } else {
            ctx.strokeRect(rect, borderColor(), borderWidth());
        }
    }
    
    // Draw children
    renderChildren(ctx);
}

void Panel::onMouseEnter() {
    Element::onMouseEnter();
}

void Panel::onMouseLeave() {
    Element::onMouseLeave();
    setisPressed(false);
}

bool Panel::onMouseEvent(const MouseEvent& event) {
    switch (event.type) {
        case MouseEventType::ButtonDown:
            if (event.button == MouseButton::Left) {
                if (pressedBackground().a > 0 || m_onClick) {
                    setisPressed(true);
                    return true;
                }
            }
            break;
        case MouseEventType::ButtonUp:
            if (event.button == MouseButton::Left && isPressed()) {
                setisPressed(false);
                if (m_onClick && containsPoint(event.position)) {
                    m_onClick();
                }
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

void Panel::renderChildren(RenderContext& ctx) {
    // Build sorted order by zIndex (stable: preserve insertion order for equal zIndex)
    std::vector<Element*> sorted;
    sorted.reserve(m_children.size());
    for (const auto& child : m_children) {
        sorted.push_back(child.get());
    }
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const Element* a, const Element* b) {
                         return a->zIndex() < b->zIndex();
                     });
    for (auto* child : sorted) {
        // Draw drop shadow for panels that have shadow properties.
        // Drawn here in the PARENT's coordinate space so it extends
        // beyond the child's own clip bounds naturally.
        if (auto* panel = dynamic_cast<Panel*>(child)) {
            if (panel->visibility() == Visibility::Visible &&
                panel->shadowColor().a > 0 &&
                (panel->shadowBlurRadius() > 0 || panel->shadowOffsetX() != 0 || panel->shadowOffsetY() != 0)) {
                Rectf childBounds = panel->bounds();
                Rectf shadowRect = {childBounds.x, childBounds.y,
                                    childBounds.width, childBounds.height};
                ctx.drawDropShadow(shadowRect, panel->cornerRadius(),
                                   panel->shadowColor(), panel->shadowBlurRadius(),
                                   panel->shadowOffsetX(), panel->shadowOffsetY());
            }
        }
        child->render(ctx);
    }
}

} // namespace gut


// --- elements/StackPanel.cpp ---

#include <algorithm>
#include <cmath>

namespace gut {

StackPanel::StackPanel(Orientation orient) {
    setorientation(orient);
}

StackPanel::StackPanel(Orientation orient, std::initializer_list<Ref<Element>> children)
    : Panel(children)
{
    setorientation(orient);
}

Size2f StackPanel::measureOverride(Size2f availableSize) {
    const bool isHorizontal = orientation() == Orientation::Horizontal;
    const f32 gap = spacing();
    
    f32 totalMain = 0;
    f32 maxCross = 0;
    
    for (usize i = 0; i < m_children.size(); ++i) {
        auto& child = m_children[i];
        
        // Measure child with unlimited size in main direction
        Size2f childAvailable = availableSize;
        if (isHorizontal) {
            childAvailable.width = INFINITY;
        } else {
            childAvailable.height = INFINITY;
        }
        
        child->measure(childAvailable);
        Size2f childDesired = child->desiredSize();
        
        if (isHorizontal) {
            totalMain += childDesired.width;
            maxCross = std::max(maxCross, childDesired.height);
        } else {
            totalMain += childDesired.height;
            maxCross = std::max(maxCross, childDesired.width);
        }
        
        // Add spacing between children
        if (i < m_children.size() - 1) {
            totalMain += gap;
        }
    }
    
    if (isHorizontal) {
        return {totalMain, maxCross};
    } else {
        return {maxCross, totalMain};
    }
}

Size2f StackPanel::arrangeOverride(Size2f finalSize) {
    const bool isHorizontal = orientation() == Orientation::Horizontal;
    const f32 gap = spacing();
    
    f32 offset = 0;
    
    for (usize i = 0; i < m_children.size(); ++i) {
        auto& child = m_children[i];
        Size2f childDesired = child->desiredSize();
        
        Rectf childRect;
        if (isHorizontal) {
            childRect = {offset, 0, childDesired.width, finalSize.height};
            offset += childDesired.width + gap;
        } else {
            childRect = {0, offset, finalSize.width, childDesired.height};
            offset += childDesired.height + gap;
        }
        
        child->arrange(childRect);
    }
    
    return finalSize;
}

} // namespace gut


// --- elements/Grid.cpp ---

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace gut {

Grid::Grid(usize rows, usize columns) {
    m_rows.resize(rows);
    m_columns.resize(columns);
}

void Grid::setRowDefinitions(std::vector<RowDefinition> rows) {
    m_rows = std::move(rows);
    invalidateLayout();
}

void Grid::setColumnDefinitions(std::vector<ColumnDefinition> columns) {
    m_columns = std::move(columns);
    invalidateLayout();
}

void Grid::setRow(Element& element, usize row) {
    const_cast<Element&>(element).setAttachedProperty("Grid.Row", row);
}

usize Grid::getRow(const Element& element) {
    return element.getAttachedProperty<usize>("Grid.Row", 0);
}

void Grid::setColumn(Element& element, usize column) {
    const_cast<Element&>(element).setAttachedProperty("Grid.Column", column);
}

usize Grid::getColumn(const Element& element) {
    return element.getAttachedProperty<usize>("Grid.Column", 0);
}

void Grid::setRowSpan(Element& element, usize span) {
    const_cast<Element&>(element).setAttachedProperty("Grid.RowSpan", span);
}

usize Grid::getRowSpan(const Element& element) {
    return element.getAttachedProperty<usize>("Grid.RowSpan", 1);
}

void Grid::setColumnSpan(Element& element, usize span) {
    const_cast<Element&>(element).setAttachedProperty("Grid.ColumnSpan", span);
}

usize Grid::getColumnSpan(const Element& element) {
    return element.getAttachedProperty<usize>("Grid.ColumnSpan", 1);
}

Size2f Grid::measureOverride(Size2f availableSize) {
    if (m_rows.empty()) m_rows.push_back({});
    if (m_columns.empty()) m_columns.push_back({});
    
    // First pass: measure all children
    for (auto& child : m_children) {
        child->measure(availableSize);
    }
    
    // Compute row heights and column widths
    computeRowHeights(availableSize.height);
    computeColumnWidths(availableSize.width);
    
    // Sum up total size
    f32 totalWidth = 0;
    f32 totalHeight = 0;
    
    for (const auto& col : m_columns) {
        totalWidth += col.actualWidth;
    }
    for (const auto& row : m_rows) {
        totalHeight += row.actualHeight;
    }
    
    return {totalWidth, totalHeight};
}

Size2f Grid::arrangeOverride(Size2f finalSize) {
    computeRowHeights(finalSize.height);
    computeColumnWidths(finalSize.width);
    
    for (auto& child : m_children) {
        usize row = getRow(*child);
        usize col = getColumn(*child);
        usize rowSpan = getRowSpan(*child);
        usize colSpan = getColumnSpan(*child);
        
        Rectf cellBounds = getCellBounds(row, col, rowSpan, colSpan);
        child->arrange(cellBounds);
    }
    
    return finalSize;
}

void Grid::computeRowHeights(f32 availableHeight) {
    f32 totalFixed = 0;
    f32 totalStarWeight = 0;
    
    // First pass: compute auto and pixel sizes
    for (auto& row : m_rows) {
        switch (row.height.type) {
            case GridLength::Type::Pixel:
                row.actualHeight = std::clamp(row.height.value, row.minHeight, row.maxHeight);
                totalFixed += row.actualHeight;
                break;
            case GridLength::Type::Auto: {
                f32 maxHeight = 0;
                for (const auto& child : m_children) {
                    if (getRow(*child) == static_cast<usize>(&row - m_rows.data())) {
                        maxHeight = std::max(maxHeight, child->desiredSize().height);
                    }
                }
                row.actualHeight = std::clamp(maxHeight, row.minHeight, row.maxHeight);
                totalFixed += row.actualHeight;
                break;
            }
            case GridLength::Type::Star:
                totalStarWeight += row.height.value;
                break;
        }
    }
    
    // Second pass: distribute remaining space to star rows
    f32 remainingHeight = std::max(0.0f, availableHeight - totalFixed);
    if (totalStarWeight > 0) {
        for (auto& row : m_rows) {
            if (row.height.type == GridLength::Type::Star) {
                f32 proportion = row.height.value / totalStarWeight;
                row.actualHeight = std::clamp(remainingHeight * proportion, row.minHeight, row.maxHeight);
            }
        }
    }
}

void Grid::computeColumnWidths(f32 availableWidth) {
    f32 totalFixed = 0;
    f32 totalStarWeight = 0;
    
    // First pass: compute auto and pixel sizes
    for (auto& col : m_columns) {
        switch (col.width.type) {
            case GridLength::Type::Pixel:
                col.actualWidth = std::clamp(col.width.value, col.minWidth, col.maxWidth);
                totalFixed += col.actualWidth;
                break;
            case GridLength::Type::Auto: {
                f32 maxWidth = 0;
                for (const auto& child : m_children) {
                    if (getColumn(*child) == static_cast<usize>(&col - m_columns.data())) {
                        maxWidth = std::max(maxWidth, child->desiredSize().width);
                    }
                }
                col.actualWidth = std::clamp(maxWidth, col.minWidth, col.maxWidth);
                totalFixed += col.actualWidth;
                break;
            }
            case GridLength::Type::Star:
                totalStarWeight += col.width.value;
                break;
        }
    }
    
    // Second pass: distribute remaining space to star columns
    f32 remainingWidth = std::max(0.0f, availableWidth - totalFixed);
    if (totalStarWeight > 0) {
        for (auto& col : m_columns) {
            if (col.width.type == GridLength::Type::Star) {
                f32 proportion = col.width.value / totalStarWeight;
                col.actualWidth = std::clamp(remainingWidth * proportion, col.minWidth, col.maxWidth);
            }
        }
    }
}

Rectf Grid::getCellBounds(usize row, usize column, usize rowSpan, usize colSpan) const {
    f32 x = 0;
    f32 y = 0;
    f32 width = 0;
    f32 height = 0;
    
    // Compute x position
    for (usize c = 0; c < column && c < m_columns.size(); ++c) {
        x += m_columns[c].actualWidth;
    }
    
    // Compute y position
    for (usize r = 0; r < row && r < m_rows.size(); ++r) {
        y += m_rows[r].actualHeight;
    }
    
    // Compute width (spanning columns)
    for (usize c = column; c < column + colSpan && c < m_columns.size(); ++c) {
        width += m_columns[c].actualWidth;
    }
    
    // Compute height (spanning rows)
    for (usize r = row; r < row + rowSpan && r < m_rows.size(); ++r) {
        height += m_rows[r].actualHeight;
    }
    
    return {x, y, width, height};
}

} // namespace gut


// --- elements/Canvas.cpp ---

#include <unordered_map>
#include <cmath>

namespace gut {

void Canvas::setLeft(Element& element, f32 value) {
    const_cast<Element&>(element).setAttachedProperty("Canvas.Left", value);
}

f32 Canvas::getLeft(const Element& element) {
    return element.getAttachedProperty<f32>("Canvas.Left", NAN);
}

void Canvas::setTop(Element& element, f32 value) {
    const_cast<Element&>(element).setAttachedProperty("Canvas.Top", value);
}

f32 Canvas::getTop(const Element& element) {
    return element.getAttachedProperty<f32>("Canvas.Top", NAN);
}

void Canvas::setRight(Element& element, f32 value) {
    const_cast<Element&>(element).setAttachedProperty("Canvas.Right", value);
}

f32 Canvas::getRight(const Element& element) {
    return element.getAttachedProperty<f32>("Canvas.Right", NAN);
}

void Canvas::setBottom(Element& element, f32 value) {
    const_cast<Element&>(element).setAttachedProperty("Canvas.Bottom", value);
}

f32 Canvas::getBottom(const Element& element) {
    return element.getAttachedProperty<f32>("Canvas.Bottom", NAN);
}

Size2f Canvas::measureOverride(Size2f availableSize) {
    // Measure all children with infinite available space
    for (auto& child : m_children) {
        child->measure({INFINITY, INFINITY});
    }
    
    // Canvas doesn't constrain its size based on children
    // It just takes whatever space is available
    return availableSize;
}

Size2f Canvas::arrangeOverride(Size2f finalSize) {
    for (auto& child : m_children) {
        f32 left = getLeft(*child);
        f32 top = getTop(*child);
        f32 right = getRight(*child);
        f32 bottom = getBottom(*child);
        
        Size2f childDesired = child->desiredSize();
        
        f32 x = 0;
        f32 y = 0;
        f32 width = childDesired.width;
        f32 height = childDesired.height;
        
        // Position based on attached properties
        if (!std::isnan(left)) {
            x = left;
            if (!std::isnan(right)) {
                width = finalSize.width - left - right;
            }
        } else if (!std::isnan(right)) {
            x = finalSize.width - right - width;
        }
        
        if (!std::isnan(top)) {
            y = top;
            if (!std::isnan(bottom)) {
                height = finalSize.height - top - bottom;
            }
        } else if (!std::isnan(bottom)) {
            y = finalSize.height - bottom - height;
        }
        
        child->arrange({x, y, width, height});
    }
    
    return finalSize;
}

} // namespace gut


// --- elements/WrapPanel.cpp ---

#include <algorithm>
#include <cmath>

namespace gut {

WrapPanel::WrapPanel(Orientation orient) {
    setorientation(orient);
}

Size2f WrapPanel::measureOverride(Size2f availableSize) {
    const bool isHorizontal = orientation() == Orientation::Horizontal;
    const f32 iGap = itemSpacing();
    const f32 lGap = lineSpacing();

    // Available extent along the main axis (the dimension we wrap on)
    const f32 mainLimit = isHorizontal ? availableSize.width : availableSize.height;

    f32 lineMain  = 0;   // accumulated main-axis for current line
    f32 lineCross = 0;   // max cross-axis in current line
    f32 totalCross = 0;  // accumulated cross-axis across all lines
    f32 maxMain    = 0;  // widest line
    bool firstInLine = true;
    bool firstLine   = true;

    for (auto& child : m_children) {
        child->measure(availableSize);
        Size2f cs = child->desiredSize();

        f32 childMain  = isHorizontal ? cs.width  : cs.height;
        f32 childCross = isHorizontal ? cs.height : cs.width;

        // Should this child go on a new line?
        f32 neededMain = firstInLine ? childMain : (iGap + childMain);
        if (!firstInLine && (lineMain + neededMain) > mainLimit) {
            // Finish current line
            maxMain = std::max(maxMain, lineMain);
            totalCross += (firstLine ? 0 : lGap) + lineCross;
            firstLine = false;
            lineMain  = childMain;
            lineCross = childCross;
            firstInLine = false;
        } else {
            lineMain += (firstInLine ? 0 : iGap) + childMain;
            lineCross = std::max(lineCross, childCross);
            firstInLine = false;
        }
    }

    // Last line
    maxMain = std::max(maxMain, lineMain);
    totalCross += (firstLine ? 0 : lGap) + lineCross;

    if (isHorizontal) return {maxMain, totalCross};
    else              return {totalCross, maxMain};
}

Size2f WrapPanel::arrangeOverride(Size2f finalSize) {
    const bool isHorizontal = orientation() == Orientation::Horizontal;
    const f32 iGap = itemSpacing();
    const f32 lGap = lineSpacing();
    const f32 mainLimit = isHorizontal ? finalSize.width : finalSize.height;

    // --- First pass: bucket children into lines ---
    struct Line {
        usize start = 0;
        usize count = 0;
        f32 mainUsed = 0;
        f32 crossSize = 0;
    };
    std::vector<Line> lines;
    {
        Line cur;
        cur.start = 0;
        bool firstInLine = true;
        for (usize i = 0; i < m_children.size(); ++i) {
            Size2f cs = m_children[i]->desiredSize();
            f32 childMain  = isHorizontal ? cs.width  : cs.height;
            f32 childCross = isHorizontal ? cs.height : cs.width;
            f32 needed = firstInLine ? childMain : (iGap + childMain);

            if (!firstInLine && (cur.mainUsed + needed) > mainLimit) {
                lines.push_back(cur);
                cur = {};
                cur.start = i;
                cur.mainUsed  = childMain;
                cur.crossSize = childCross;
                cur.count     = 1;
                firstInLine   = false;
            } else {
                cur.mainUsed += (firstInLine ? 0 : iGap) + childMain;
                cur.crossSize = std::max(cur.crossSize, childCross);
                cur.count++;
                firstInLine = false;
            }
        }
        if (cur.count > 0) lines.push_back(cur);
    }

    // --- Second pass: arrange each line ---
    f32 crossOffset = 0;
    for (auto& line : lines) {
        f32 mainOffset = 0;
        for (usize i = line.start; i < line.start + line.count; ++i) {
            Size2f cs = m_children[i]->desiredSize();
            f32 childMain  = isHorizontal ? cs.width  : cs.height;

            Rectf rect;
            if (isHorizontal) {
                rect = {mainOffset, crossOffset, childMain, line.crossSize};
            } else {
                rect = {crossOffset, mainOffset, line.crossSize, childMain};
            }
            m_children[i]->arrange(rect);
            mainOffset += childMain + iGap;
        }
        crossOffset += line.crossSize + lGap;
    }

    return finalSize;
}

} // namespace gut


// --- elements/DockPanel.cpp ---

#include <algorithm>
#include <cmath>

namespace gut {

void DockPanel::setDock(Element& element, Dock value) {
    element.setAttachedProperty("DockPanel.Dock", static_cast<u8>(value));
}

Dock DockPanel::getDock(const Element& element) {
    return static_cast<Dock>(element.getAttachedProperty<u8>("DockPanel.Dock", static_cast<u8>(Dock::Left)));
}

Size2f DockPanel::measureOverride(Size2f availableSize) {
    f32 leftAccum = 0, topAccum = 0, rightAccum = 0, bottomAccum = 0;
    f32 maxWidth = 0, maxHeight = 0;

    for (usize i = 0; i < m_children.size(); ++i) {
        auto& child = m_children[i];
        Dock dock = getDock(*child);

        // Remaining available space
        f32 remainW = std::max(0.0f, availableSize.width  - leftAccum - rightAccum);
        f32 remainH = std::max(0.0f, availableSize.height - topAccum  - bottomAccum);
        child->measure({remainW, remainH});
        Size2f cs = child->desiredSize();

        switch (dock) {
        case Dock::Left:
            maxHeight = std::max(maxHeight, topAccum + bottomAccum + cs.height);
            leftAccum += cs.width;
            break;
        case Dock::Right:
            maxHeight = std::max(maxHeight, topAccum + bottomAccum + cs.height);
            rightAccum += cs.width;
            break;
        case Dock::Top:
            maxWidth = std::max(maxWidth, leftAccum + rightAccum + cs.width);
            topAccum += cs.height;
            break;
        case Dock::Bottom:
            maxWidth = std::max(maxWidth, leftAccum + rightAccum + cs.width);
            bottomAccum += cs.height;
            break;
        }
    }

    maxWidth  = std::max(maxWidth,  leftAccum + rightAccum);
    maxHeight = std::max(maxHeight, topAccum  + bottomAccum);
    return {maxWidth, maxHeight};
}

Size2f DockPanel::arrangeOverride(Size2f finalSize) {
    f32 leftAccum = 0, topAccum = 0, rightAccum = 0, bottomAccum = 0;

    usize count = m_children.size();
    for (usize i = 0; i < count; ++i) {
        auto& child = m_children[i];
        Size2f cs = child->desiredSize();

        f32 remainW = std::max(0.0f, finalSize.width  - leftAccum - rightAccum);
        f32 remainH = std::max(0.0f, finalSize.height - topAccum  - bottomAccum);

        // Last child fills remaining space if enabled
        bool isLast = (i == count - 1);
        if (isLast && lastChildFill()) {
            child->arrange({leftAccum, topAccum, remainW, remainH});
            break;
        }

        Dock dock = getDock(*child);
        Rectf rect;
        switch (dock) {
        case Dock::Left:
            rect = {leftAccum, topAccum, cs.width, remainH};
            leftAccum += cs.width;
            break;
        case Dock::Right:
            rect = {finalSize.width - rightAccum - cs.width, topAccum, cs.width, remainH};
            rightAccum += cs.width;
            break;
        case Dock::Top:
            rect = {leftAccum, topAccum, remainW, cs.height};
            topAccum += cs.height;
            break;
        case Dock::Bottom:
            rect = {leftAccum, finalSize.height - bottomAccum - cs.height, remainW, cs.height};
            bottomAccum += cs.height;
            break;
        }
        child->arrange(rect);
    }

    return finalSize;
}

} // namespace gut


// --- elements/ViewBox.cpp ---

#include <algorithm>
#include <cmath>

namespace gut {

Size2f ViewBox::measureOverride(Size2f availableSize) {
    if (m_children.empty()) return {0, 0};

    // Measure the (first) child at infinite space to get its natural size
    auto& child = m_children[0];
    child->measure({INFINITY, INFINITY});
    Size2f childNatural = child->desiredSize();

    if (childNatural.width <= 0 || childNatural.height <= 0) return {0, 0};

    // The ViewBox itself wants the constrained version of the child's natural aspect ratio
    f32 aspect = childNatural.width / childNatural.height;

    // If available size is finite, constrain to it keeping aspect
    f32 w = availableSize.width;
    f32 h = availableSize.height;
    bool wInf = std::isinf(w);
    bool hInf = std::isinf(h);

    if (wInf && hInf) return childNatural;
    if (wInf) return {h * aspect, h};
    if (hInf) return {w, w / aspect};

    // Both finite — fit to available
    f32 scaleW = w / childNatural.width;
    f32 scaleH = h / childNatural.height;
    f32 scale  = std::min(scaleW, scaleH);
    return {childNatural.width * scale, childNatural.height * scale};
}

Size2f ViewBox::arrangeOverride(Size2f finalSize) {
    if (m_children.empty()) {
        m_scaleX = m_scaleY = 1.0f;
        m_offsetX = m_offsetY = 0.0f;
        return finalSize;
    }

    auto& child = m_children[0];
    Size2f childNatural = child->desiredSize();
    if (childNatural.width <= 0 || childNatural.height <= 0) {
        m_scaleX = m_scaleY = 1.0f;
        m_offsetX = m_offsetY = 0.0f;
        child->arrange({0, 0, finalSize.width, finalSize.height});
        return finalSize;
    }

    switch (stretch()) {
    case ViewBoxStretch::None:
        m_scaleX = m_scaleY = 1.0f;
        break;
    case ViewBoxStretch::Fill:
        m_scaleX = finalSize.width  / childNatural.width;
        m_scaleY = finalSize.height / childNatural.height;
        break;
    case ViewBoxStretch::Uniform: {
        f32 s = std::min(finalSize.width / childNatural.width,
                         finalSize.height / childNatural.height);
        m_scaleX = m_scaleY = s;
        break;
    }
    case ViewBoxStretch::UniformToFill: {
        f32 s = std::max(finalSize.width / childNatural.width,
                         finalSize.height / childNatural.height);
        m_scaleX = m_scaleY = s;
        break;
    }
    }

    f32 scaledW = childNatural.width  * m_scaleX;
    f32 scaledH = childNatural.height * m_scaleY;

    // Center the content
    m_offsetX = (finalSize.width  - scaledW) * 0.5f;
    m_offsetY = (finalSize.height - scaledH) * 0.5f;

    // Arrange the child at its natural size; renderChildren will scale
    child->arrange({0, 0, childNatural.width, childNatural.height});

    return finalSize;
}

void ViewBox::renderChildren(RenderContext& ctx) {
    if (m_children.empty()) return;

    ctx.save();
    ctx.translate(m_offsetX, m_offsetY);
    ctx.scale(m_scaleX, m_scaleY);

    // Render only the first child
    m_children[0]->render(ctx);

    ctx.restore();
}

} // namespace gut


// --- elements/AnchorPanel.cpp ---

#include <algorithm>
#include <cmath>

namespace gut {

// --- Anchor ratio attached properties ---

void AnchorPanel::setAnchorLeft(Element& element, f32 value)   { element.setAttachedProperty("AnchorPanel.AnchorLeft",   value); }
f32  AnchorPanel::getAnchorLeft(const Element& element)        { return element.getAttachedProperty<f32>("AnchorPanel.AnchorLeft",   0.0f); }

void AnchorPanel::setAnchorTop(Element& element, f32 value)    { element.setAttachedProperty("AnchorPanel.AnchorTop",    value); }
f32  AnchorPanel::getAnchorTop(const Element& element)         { return element.getAttachedProperty<f32>("AnchorPanel.AnchorTop",    0.0f); }

void AnchorPanel::setAnchorRight(Element& element, f32 value)  { element.setAttachedProperty("AnchorPanel.AnchorRight",  value); }
f32  AnchorPanel::getAnchorRight(const Element& element)       { return element.getAttachedProperty<f32>("AnchorPanel.AnchorRight",  1.0f); }

void AnchorPanel::setAnchorBottom(Element& element, f32 value) { element.setAttachedProperty("AnchorPanel.AnchorBottom", value); }
f32  AnchorPanel::getAnchorBottom(const Element& element)      { return element.getAttachedProperty<f32>("AnchorPanel.AnchorBottom", 1.0f); }

// --- Offset (pixels) attached properties ---

void AnchorPanel::setOffsetLeft(Element& element, f32 value)   { element.setAttachedProperty("AnchorPanel.OffsetLeft",   value); }
f32  AnchorPanel::getOffsetLeft(const Element& element)        { return element.getAttachedProperty<f32>("AnchorPanel.OffsetLeft",   NAN); }

void AnchorPanel::setOffsetTop(Element& element, f32 value)    { element.setAttachedProperty("AnchorPanel.OffsetTop",    value); }
f32  AnchorPanel::getOffsetTop(const Element& element)         { return element.getAttachedProperty<f32>("AnchorPanel.OffsetTop",    NAN); }

void AnchorPanel::setOffsetRight(Element& element, f32 value)  { element.setAttachedProperty("AnchorPanel.OffsetRight",  value); }
f32  AnchorPanel::getOffsetRight(const Element& element)       { return element.getAttachedProperty<f32>("AnchorPanel.OffsetRight",  NAN); }

void AnchorPanel::setOffsetBottom(Element& element, f32 value) { element.setAttachedProperty("AnchorPanel.OffsetBottom", value); }
f32  AnchorPanel::getOffsetBottom(const Element& element)      { return element.getAttachedProperty<f32>("AnchorPanel.OffsetBottom", NAN); }

Size2f AnchorPanel::measureOverride(Size2f availableSize) {
    // Measure all children at infinite — they are positioned absolutely
    for (auto& child : m_children) {
        child->measure({INFINITY, INFINITY});
    }
    return availableSize;
}

Size2f AnchorPanel::arrangeOverride(Size2f finalSize) {
    const f32 pw = finalSize.width;
    const f32 ph = finalSize.height;

    for (auto& child : m_children) {
        Size2f cs = child->desiredSize();

        f32 anchorL = getAnchorLeft(*child);
        f32 anchorT = getAnchorTop(*child);
        f32 anchorR = getAnchorRight(*child);
        f32 anchorB = getAnchorBottom(*child);

        f32 offL = getOffsetLeft(*child);
        f32 offT = getOffsetTop(*child);
        f32 offR = getOffsetRight(*child);
        f32 offB = getOffsetBottom(*child);

        bool hasL = !std::isnan(offL);
        bool hasT = !std::isnan(offT);
        bool hasR = !std::isnan(offR);
        bool hasB = !std::isnan(offB);

        f32 x, y, w, h;

        // --- Horizontal ---
        if (hasL && hasR) {
            // Stretch between two anchors + offsets
            f32 left  = anchorL * pw + offL;
            f32 right = anchorR * pw - offR;
            x = left;
            w = std::max(0.0f, right - left);
        } else if (hasL) {
            x = anchorL * pw + offL;
            w = cs.width;
        } else if (hasR) {
            x = anchorR * pw - offR - cs.width;
            w = cs.width;
        } else {
            // No offset set — just place at anchor position
            x = anchorL * pw;
            w = cs.width;
        }

        // --- Vertical ---
        if (hasT && hasB) {
            f32 top    = anchorT * ph + offT;
            f32 bottom = anchorB * ph - offB;
            y = top;
            h = std::max(0.0f, bottom - top);
        } else if (hasT) {
            y = anchorT * ph + offT;
            h = cs.height;
        } else if (hasB) {
            y = anchorB * ph - offB - cs.height;
            h = cs.height;
        } else {
            y = anchorT * ph;
            h = cs.height;
        }

        child->arrange({x, y, w, h});
    }

    return finalSize;
}

} // namespace gut


// --- elements/UniformGrid.cpp ---

namespace gut {

void UniformGrid::computeGrid(i32& outCols, i32& outRows) const {
    i32 count = static_cast<i32>(m_children.size());
    i32 c = columns();
    i32 r = rows();

    if (c > 0 && r > 0) {
        outCols = c;
        outRows = r;
    } else if (c > 0) {
        outCols = c;
        outRows = (count + c - 1) / c;
    } else if (r > 0) {
        outRows = r;
        outCols = (count + r - 1) / r;
    } else {
        // Auto: square-ish grid
        outCols = std::max(1, (i32)std::ceil(std::sqrt((f32)count)));
        outRows = (count + outCols - 1) / outCols;
    }
    if (outCols < 1) outCols = 1;
    if (outRows < 1) outRows = 1;
}

Size2f UniformGrid::measureOverride(Size2f availableSize) {
    i32 cols, rws;
    computeGrid(cols, rws);

    // Measure every child to find the largest cell
    f32 maxW = 0, maxH = 0;
    Size2f cellAvail = { availableSize.width / cols, availableSize.height / rws };
    for (auto& child : m_children) {
        child->measure(cellAvail);
        Size2f ds = child->desiredSize();
        maxW = std::max(maxW, ds.width);
        maxH = std::max(maxH, ds.height);
    }
    return { maxW * cols, maxH * rws };
}

Size2f UniformGrid::arrangeOverride(Size2f finalSize) {
    i32 cols, rws;
    computeGrid(cols, rws);

    f32 cellW = finalSize.width  / cols;
    f32 cellH = finalSize.height / rws;

    for (usize i = 0; i < m_children.size(); ++i) {
        i32 col = static_cast<i32>(i) % cols;
        i32 row = static_cast<i32>(i) / cols;
        m_children[i]->arrange({ col * cellW, row * cellH, cellW, cellH });
    }
    return finalSize;
}

} // namespace gut


// --- elements/FlexPanel.cpp ---

namespace gut {

FlexPanel::FlexPanel(Orientation orient) {
    setorientation(orient);
}

void FlexPanel::setFlexGrow(Element& el, f32 v)   { el.setAttachedProperty("Flex.Grow", v); }
f32  FlexPanel::getFlexGrow(const Element& el)    { return el.getAttachedProperty<f32>("Flex.Grow", 0.0f); }
void FlexPanel::setFlexShrink(Element& el, f32 v) { el.setAttachedProperty("Flex.Shrink", v); }
f32  FlexPanel::getFlexShrink(const Element& el)  { return el.getAttachedProperty<f32>("Flex.Shrink", 1.0f); }
void FlexPanel::setFlexBasis(Element& el, f32 v)  { el.setAttachedProperty("Flex.Basis", v); }
f32  FlexPanel::getFlexBasis(const Element& el)   { return el.getAttachedProperty<f32>("Flex.Basis", NAN); }

void FlexPanel::setAlignSelf(Element& el, FlexAlign v) {
    el.setAttachedProperty("Flex.AlignSelf", static_cast<u8>(v));
}
FlexAlign FlexPanel::getAlignSelf(const Element& el) {
    // 255 = sentinel for "inherit from alignItems"
    u8 raw = el.getAttachedProperty<u8>("Flex.AlignSelf", 255);
    return raw == 255 ? FlexAlign::Stretch : static_cast<FlexAlign>(raw);
}

Size2f FlexPanel::measureOverride(Size2f availableSize) {
    const bool horiz = orientation() == Orientation::Horizontal;
    const f32 gap = spacing();
    const usize n = m_children.size();

    f32 totalMain = 0;
    f32 maxCross  = 0;

    for (usize i = 0; i < n; ++i) {
        auto& child = m_children[i];
        child->measure(availableSize);
        Size2f ds = child->desiredSize();

        f32 basis = getFlexBasis(*child);
        f32 mainSize = std::isnan(basis) ? (horiz ? ds.width : ds.height) : basis;
        f32 crossSize = horiz ? ds.height : ds.width;

        totalMain += mainSize;
        if (i > 0) totalMain += gap;
        maxCross = std::max(maxCross, crossSize);
    }

    return horiz ? Size2f{totalMain, maxCross} : Size2f{maxCross, totalMain};
}

Size2f FlexPanel::arrangeOverride(Size2f finalSize) {
    const bool horiz = orientation() == Orientation::Horizontal;
    const f32 gap = spacing();
    const usize n = m_children.size();
    if (n == 0) return finalSize;

    const f32 mainExtent = horiz ? finalSize.width : finalSize.height;
    const f32 crossExtent = horiz ? finalSize.height : finalSize.width;

    // Collect natural main sizes via basis or desired
    struct Item {
        f32 base;    // basis or natural
        f32 grow;
        f32 shrink;
        f32 final_;  // computed main size
        FlexAlign align;
    };
    std::vector<Item> items(n);
    f32 totalBase = 0;
    f32 totalGrow = 0;
    f32 totalShrink = 0;

    for (usize i = 0; i < n; ++i) {
        auto& child = m_children[i];
        Size2f ds = child->desiredSize();
        f32 basis = getFlexBasis(*child);
        f32 natural = horiz ? ds.width : ds.height;
        items[i].base   = std::isnan(basis) ? natural : basis;
        items[i].grow   = getFlexGrow(*child);
        items[i].shrink = getFlexShrink(*child);
        // Check for explicit alignSelf
        u8 raw = child->getAttachedProperty<u8>("Flex.AlignSelf", 255);
        items[i].align = (raw == 255) ? alignItems() : static_cast<FlexAlign>(raw);
        totalBase += items[i].base;
        totalGrow += items[i].grow;
        totalShrink += items[i].shrink;
    }

    f32 totalGaps = gap * (f32)(n - 1);
    f32 freeSpace = mainExtent - totalBase - totalGaps;

    // Distribute free space
    for (usize i = 0; i < n; ++i) {
        items[i].final_ = items[i].base;
        if (freeSpace > 0 && totalGrow > 0) {
            items[i].final_ += freeSpace * (items[i].grow / totalGrow);
        } else if (freeSpace < 0 && totalShrink > 0) {
            items[i].final_ += freeSpace * (items[i].shrink / totalShrink);
        }
        items[i].final_ = std::max(0.0f, items[i].final_);
    }

    // Compute total used for justify
    f32 totalUsed = totalGaps;
    for (auto& it : items) totalUsed += it.final_;
    f32 remaining = mainExtent - totalUsed;

    // Justify offsets
    f32 startOffset = 0;
    f32 extraGap = 0;
    switch (justifyContent()) {
        case FlexJustify::Start:        break;
        case FlexJustify::End:          startOffset = remaining; break;
        case FlexJustify::Center:       startOffset = remaining * 0.5f; break;
        case FlexJustify::SpaceBetween:
            extraGap = (n > 1) ? remaining / (f32)(n - 1) : 0;
            break;
        case FlexJustify::SpaceAround:
            extraGap = remaining / (f32)n;
            startOffset = extraGap * 0.5f;
            break;
        case FlexJustify::SpaceEvenly:
            extraGap = remaining / (f32)(n + 1);
            startOffset = extraGap;
            break;
    }

    f32 mainPos = startOffset;
    for (usize i = 0; i < n; ++i) {
        auto& child = m_children[i];
        f32 childMain  = items[i].final_;
        f32 childCross = crossExtent;
        f32 crossPos   = 0;

        FlexAlign align = items[i].align;
        if (align != FlexAlign::Stretch) {
            Size2f ds = child->desiredSize();
            f32 naturalCross = horiz ? ds.height : ds.width;
            childCross = naturalCross;
            switch (align) {
                case FlexAlign::Start:   crossPos = 0; break;
                case FlexAlign::End:     crossPos = crossExtent - naturalCross; break;
                case FlexAlign::Center:  crossPos = (crossExtent - naturalCross) * 0.5f; break;
                default: break;
            }
        }

        Rectf rect;
        if (horiz) {
            rect = {mainPos, crossPos, childMain, childCross};
        } else {
            rect = {crossPos, mainPos, childCross, childMain};
        }
        child->arrange(rect);
        mainPos += childMain + gap + extraGap;
    }

    return finalSize;
}

} // namespace gut


// --- elements/RelativePanel.cpp ---

namespace gut {

// --- Attached property helpers ---

void RelativePanel::setLeftOf(Element& el, const String& s)    { el.setAttachedProperty("RP.LeftOf", s); }
String RelativePanel::getLeftOf(const Element& el)             { return el.getAttachedProperty<String>("RP.LeftOf", String{}); }
void RelativePanel::setRightOf(Element& el, const String& s)   { el.setAttachedProperty("RP.RightOf", s); }
String RelativePanel::getRightOf(const Element& el)            { return el.getAttachedProperty<String>("RP.RightOf", String{}); }
void RelativePanel::setAbove(Element& el, const String& s)     { el.setAttachedProperty("RP.Above", s); }
String RelativePanel::getAbove(const Element& el)              { return el.getAttachedProperty<String>("RP.Above", String{}); }
void RelativePanel::setBelow(Element& el, const String& s)     { el.setAttachedProperty("RP.Below", s); }
String RelativePanel::getBelow(const Element& el)              { return el.getAttachedProperty<String>("RP.Below", String{}); }

void RelativePanel::setAlignLeftWith(Element& el, const String& s)   { el.setAttachedProperty("RP.AlignLeftWith", s); }
String RelativePanel::getAlignLeftWith(const Element& el)            { return el.getAttachedProperty<String>("RP.AlignLeftWith", String{}); }
void RelativePanel::setAlignTopWith(Element& el, const String& s)    { el.setAttachedProperty("RP.AlignTopWith", s); }
String RelativePanel::getAlignTopWith(const Element& el)             { return el.getAttachedProperty<String>("RP.AlignTopWith", String{}); }
void RelativePanel::setAlignRightWith(Element& el, const String& s)  { el.setAttachedProperty("RP.AlignRightWith", s); }
String RelativePanel::getAlignRightWith(const Element& el)           { return el.getAttachedProperty<String>("RP.AlignRightWith", String{}); }
void RelativePanel::setAlignBottomWith(Element& el, const String& s) { el.setAttachedProperty("RP.AlignBottomWith", s); }
String RelativePanel::getAlignBottomWith(const Element& el)          { return el.getAttachedProperty<String>("RP.AlignBottomWith", String{}); }

void RelativePanel::setAlignLeftWithPanel(Element& el, bool v)              { el.setAttachedProperty("RP.AlignLeftPanel", v); }
bool RelativePanel::getAlignLeftWithPanel(const Element& el)                { return el.getAttachedProperty<bool>("RP.AlignLeftPanel", false); }
void RelativePanel::setAlignTopWithPanel(Element& el, bool v)               { el.setAttachedProperty("RP.AlignTopPanel", v); }
bool RelativePanel::getAlignTopWithPanel(const Element& el)                 { return el.getAttachedProperty<bool>("RP.AlignTopPanel", false); }
void RelativePanel::setAlignRightWithPanel(Element& el, bool v)             { el.setAttachedProperty("RP.AlignRightPanel", v); }
bool RelativePanel::getAlignRightWithPanel(const Element& el)               { return el.getAttachedProperty<bool>("RP.AlignRightPanel", false); }
void RelativePanel::setAlignBottomWithPanel(Element& el, bool v)            { el.setAttachedProperty("RP.AlignBottomPanel", v); }
bool RelativePanel::getAlignBottomWithPanel(const Element& el)              { return el.getAttachedProperty<bool>("RP.AlignBottomPanel", false); }
void RelativePanel::setAlignHorizontalCenterWithPanel(Element& el, bool v)  { el.setAttachedProperty("RP.AlignHCenterPanel", v); }
bool RelativePanel::getAlignHorizontalCenterWithPanel(const Element& el)    { return el.getAttachedProperty<bool>("RP.AlignHCenterPanel", false); }
void RelativePanel::setAlignVerticalCenterWithPanel(Element& el, bool v)    { el.setAttachedProperty("RP.AlignVCenterPanel", v); }
bool RelativePanel::getAlignVerticalCenterWithPanel(const Element& el)      { return el.getAttachedProperty<bool>("RP.AlignVCenterPanel", false); }

Element* RelativePanel::findSibling(const String& name) const {
    if (name.empty()) return nullptr;
    for (auto& child : m_children) {
        if (child->id() == name) return child.get();
    }
    return nullptr;
}

Size2f RelativePanel::measureOverride(Size2f availableSize) {
    for (auto& child : m_children) {
        child->measure(availableSize);
    }
    return availableSize;
}

Size2f RelativePanel::arrangeOverride(Size2f finalSize) {
    const f32 pw = finalSize.width;
    const f32 ph = finalSize.height;

    // Build a name->arranged-rect map.  We do multiple passes to resolve
    // dependencies.  In the worst case (chain of N), N passes suffice.
    struct Slot {
        Element* el;
        Rectf rect;
        bool resolved;
    };
    std::vector<Slot> slots(m_children.size());
    for (usize i = 0; i < m_children.size(); ++i) {
        slots[i].el = m_children[i].get();
        slots[i].resolved = false;
    }

    auto findRect = [&](const String& nm) -> const Rectf* {
        for (auto& s : slots) {
            if (s.resolved && s.el->id() == nm) return &s.rect;
        }
        return nullptr;
    };

    usize maxPasses = m_children.size() + 1;
    for (usize pass = 0; pass < maxPasses; ++pass) {
        bool progress = false;
        for (auto& slot : slots) {
            if (slot.resolved) continue;
            Element& el = *slot.el;
            Size2f ds = el.desiredSize();

            // Check dependencies
            auto needSibling = [&](const String& n) -> bool {
                if (n.empty()) return false;
                return findRect(n) == nullptr;
            };
            bool blocked = false;
            auto checkBlock = [&](const String& n) { if (needSibling(n)) blocked = true; };
            checkBlock(getRightOf(el));
            checkBlock(getLeftOf(el));
            checkBlock(getBelow(el));
            checkBlock(getAbove(el));
            checkBlock(getAlignLeftWith(el));
            checkBlock(getAlignTopWith(el));
            checkBlock(getAlignRightWith(el));
            checkBlock(getAlignBottomWith(el));
            if (blocked) continue;

            f32 x = 0, y = 0, w = ds.width, h = ds.height;

            // Positional
            auto rightOfName = getRightOf(el);
            auto leftOfName  = getLeftOf(el);
            auto belowName   = getBelow(el);
            auto aboveName   = getAbove(el);

            if (!rightOfName.empty()) {
                if (auto* r = findRect(rightOfName)) x = r->x + r->width;
            }
            if (!leftOfName.empty()) {
                if (auto* r = findRect(leftOfName)) x = r->x - w;
            }
            if (!belowName.empty()) {
                if (auto* r = findRect(belowName)) y = r->y + r->height;
            }
            if (!aboveName.empty()) {
                if (auto* r = findRect(aboveName)) y = r->y - h;
            }

            // Alignment with sibling
            auto alLeft   = getAlignLeftWith(el);
            auto alTop    = getAlignTopWith(el);
            auto alRight  = getAlignRightWith(el);
            auto alBottom = getAlignBottomWith(el);

            if (!alLeft.empty()) {
                if (auto* r = findRect(alLeft)) x = r->x;
            }
            if (!alTop.empty()) {
                if (auto* r = findRect(alTop)) y = r->y;
            }
            if (!alRight.empty()) {
                if (auto* r = findRect(alRight)) x = r->x + r->width - w;
            }
            if (!alBottom.empty()) {
                if (auto* r = findRect(alBottom)) y = r->y + r->height - h;
            }

            // Panel-edge alignment
            if (getAlignLeftWithPanel(el))   x = 0;
            if (getAlignTopWithPanel(el))    y = 0;
            if (getAlignRightWithPanel(el))  x = pw - w;
            if (getAlignBottomWithPanel(el)) y = ph - h;
            if (getAlignHorizontalCenterWithPanel(el)) x = (pw - w) * 0.5f;
            if (getAlignVerticalCenterWithPanel(el))   y = (ph - h) * 0.5f;

            slot.rect = {x, y, w, h};
            slot.resolved = true;
            progress = true;
        }
        if (!progress) break;  // no more resolvable or all done
    }

    // Arrange everything (unresolved gets (0,0))
    for (auto& slot : slots) {
        if (!slot.resolved) slot.rect = {0, 0, slot.el->desiredSize().width, slot.el->desiredSize().height};
        slot.el->arrange(slot.rect);
    }

    return finalSize;
}

} // namespace gut


// --- elements/RadialPanel.cpp ---

namespace gut {

Size2f RadialPanel::measureOverride(Size2f availableSize) {
    // Measure all children with infinite space
    for (auto& child : m_children) {
        child->measure({INFINITY, INFINITY});
    }
    return availableSize;
}

Size2f RadialPanel::arrangeOverride(Size2f finalSize) {
    const usize n = m_children.size();
    if (n == 0) return finalSize;

    const f32 cx = finalSize.width  * 0.5f;
    const f32 cy = finalSize.height * 0.5f;

    f32 r = radius();
    if (r <= 0) {
        // Auto radius: half the smallest dimension, minus room for children
        f32 maxChildR = 0;
        for (auto& child : m_children) {
            Size2f ds = child->desiredSize();
            maxChildR = std::max(maxChildR, std::max(ds.width, ds.height) * 0.5f);
        }
        r = std::max(0.0f, std::min(cx, cy) - maxChildR);
    }

    constexpr f32 DEG2RAD = 3.14159265358979323846f / 180.0f;
    // Convert angles: 0deg = 12 o'clock (top), clockwise.
    // In math coords: 12 o'clock = -90deg.
    f32 startRad = (startAngle() - 90.0f) * DEG2RAD;
    f32 endRad   = (endAngle()   - 90.0f) * DEG2RAD;

    bool fullCircle = std::abs(endAngle() - startAngle()) >= 360.0f;
    f32 span = endRad - startRad;
    f32 step = (n == 1) ? 0.0f : (fullCircle ? span / (f32)n : span / (f32)(n - 1));

    for (usize i = 0; i < n; ++i) {
        f32 angle = startRad + step * (f32)i;
        f32 px = cx + r * std::cos(angle);
        f32 py = cy + r * std::sin(angle);

        Size2f ds = m_children[i]->desiredSize();
        // Center the child on the computed position
        f32 lx = px - ds.width  * 0.5f;
        f32 ly = py - ds.height * 0.5f;
        m_children[i]->arrange({lx, ly, ds.width, ds.height});
    }

    return finalSize;
}

} // namespace gut


// --- elements/Text.cpp ---


namespace gut {

Text::Text(String textContent) {
    settext(std::move(textContent));
}

Text::Text(String textContent, f32 size) {
    settext(std::move(textContent));
    setfontSize(size);
}

Size2f Text::measureOverride(Size2f availableSize) {
    // Try to use the real font system
    if (context()) {
        Font* font = context()->findFont(fontFamily(), effectiveFontWeight(), effectiveFontStyle());
        if (!font) font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(fontSize());
            if (face) {
                bool wrapping = textWrapping() != TextWrapping::NoWrap;
                f32 maxW = wrapping ? availableSize.width : 0;
                
                if (wrapping || textAlignment() != TextAlignment::Left) {
                    // Use shapeMultiline for accurate multi-line measurement
                    TextShaper shaper;
                    auto toGlobalAlign = [](TextAlignment a) -> gut::TextAlignment {
                        switch (a) {
                            case TextAlignment::Center: return gut::TextAlignment::Center;
                            case TextAlignment::Right:  return gut::TextAlignment::Right;
                            case TextAlignment::Justify: return gut::TextAlignment::Justify;
                            default: return gut::TextAlignment::Left;
                        }
                    };
                    auto lines = shaper.shapeMultiline(face.get(), text(), maxW, toGlobalAlign(textAlignment()));
                    f32 maxLineW = 0;
                    for (const auto& line : lines) {
                        maxLineW = std::max(maxLineW, line.width);
                    }
                    f32 h = static_cast<f32>(lines.size()) * face->lineHeight();
                    if (h <= 0) h = face->lineHeight();
                    return {maxLineW, h};
                }
                
                return face->measureText(text(), maxW);
            }
        }
    }
    
    // Fallback: approximate when no font is available
    f32 charWidth = fontSize() * 0.5f;
    f32 textWidth = static_cast<f32>(text().length()) * charWidth;
    f32 textHeight = fontSize() * lineHeight();
    
    if (textWrapping() != TextWrapping::NoWrap && textWidth > availableSize.width) {
        f32 numLines = textWidth / availableSize.width;
        textWidth = availableSize.width;
        textHeight *= numLines;
    }
    
    return {textWidth, textHeight};
}

// Helper: truncate a string with ellipsis so it fits within maxWidth pixels.
// Returns the original string if it already fits.
static std::string truncateWithEllipsis(FontFace* face, const std::string& text,
                                        f32 maxWidth, bool wordBoundary) {
    if (maxWidth <= 0) return text;
    
    // Measure the full text first — fast path
    f32 fullWidth = face->measureWidth(text);
    if (fullWidth <= maxWidth) return text;
    
    // Measure the ellipsis character
    static const std::string ellipsis = "\xE2\x80\xA6"; // UTF-8 "…" (U+2026)
    f32 ellipsisWidth = 0;
    {
        const Glyph* eg = face->glyph(0x2026); // U+2026 HORIZONTAL ELLIPSIS
        if (eg) {
            ellipsisWidth = eg->advance;
        } else {
            // Fallback: use three dots
            const Glyph* dotG = face->glyph('.');
            ellipsisWidth = dotG ? dotG->advance * 3 : 0;
        }
    }
    
    f32 budget = maxWidth - ellipsisWidth;
    if (budget <= 0) return ellipsis;
    
    // Walk glyph by glyph to find the cut point
    f32 w = 0;
    size_t cutAt = 0;
    size_t lastWordBreak = 0;
    u32 prevCodepoint = 0;
    
    for (size_t i = 0; i < text.size(); ++i) {
        u32 cp = static_cast<u32>(static_cast<unsigned char>(text[i]));
        const Glyph* g = face->glyph(cp);
        f32 advance = g ? g->advance : 0;
        if (prevCodepoint != 0 && g) {
            advance += face->kerning(prevCodepoint, cp);
        }
        if (w + advance > budget) break;
        w += advance;
        cutAt = i + 1;
        if (text[i] == ' ' || text[i] == '\t' || text[i] == '-') {
            lastWordBreak = cutAt;
        }
        prevCodepoint = cp;
    }
    
    if (wordBoundary && lastWordBreak > 0) {
        cutAt = lastWordBreak;
    }
    
    // Trim trailing spaces before ellipsis
    while (cutAt > 0 && text[cutAt - 1] == ' ') --cutAt;
    
    // Check if we have the Unicode ellipsis glyph
    const Glyph* eg = face->glyph(0x2026);
    std::string suffix = eg ? ellipsis : "...";
    
    return text.substr(0, cutAt) + suffix;
}

void Text::onRender(RenderContext& ctx) {
    if (context()) {
        Font* font = context()->findFont(fontFamily(), effectiveFontWeight(), effectiveFontStyle());
        if (!font) font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(fontSize());
            if (face) {
                f32 x = padding().left;
                f32 y = padding().top + face->ascender();
                f32 maxW = bounds().width - padding().horizontalSum();
                
                bool wrapping = textWrapping() != TextWrapping::NoWrap;
                bool aligned = textAlignment() != TextAlignment::Left;
                bool trimming = textTrimming() != TextTrimming::None;
                
                // Determine render text — apply ellipsis truncation for single-line
                std::string renderText = text();
                if (trimming && !wrapping && maxW > 0) {
                    bool wordBreak = (textTrimming() == TextTrimming::WordEllipsis);
                    renderText = truncateWithEllipsis(face.get(), renderText, maxW, wordBreak);
                }
                
                auto toGlobalAlign = [](TextAlignment a) -> gut::TextAlignment {
                    switch (a) {
                        case TextAlignment::Center: return gut::TextAlignment::Center;
                        case TextAlignment::Right:  return gut::TextAlignment::Right;
                        case TextAlignment::Justify: return gut::TextAlignment::Justify;
                        default: return gut::TextAlignment::Left;
                    }
                };
                
                // Helper lambda: draw the text at a given position with a given colour
                auto drawPass = [&](f32 ox, f32 oy, Color col) {
                    if (wrapping || aligned) {
                        ctx.drawTextMultiline(face.get(), renderText, {x + ox, y + oy}, col,
                                              maxW, toGlobalAlign(textAlignment()));
                    } else {
                        ctx.drawText(face.get(), renderText, {x + ox, y + oy}, col);
                    }
                };
                
                // ---- Pass 1: Glow (outermost layer) ----
                if (textGlowColor().a > 0 && textGlowRadius() > 0) {
                    const int rings = std::max(2, static_cast<int>(textGlowRadius()));
                    const int samples = 12;  // angular samples per ring
                    f32 baseAlpha = static_cast<f32>(textGlowColor().a) / 255.0f;
                    for (int r = 1; r <= rings; r++) {
                        f32 t = static_cast<f32>(r) / static_cast<f32>(rings);
                        f32 radius = t * textGlowRadius();
                        f32 alpha = baseAlpha * (1.0f - t * 0.7f) / static_cast<f32>(rings);
                        Color gc = Color::fromRgba8(textGlowColor().r, textGlowColor().g, textGlowColor().b,
                                                    static_cast<u8>(std::min(255.0f, alpha * 255.0f)));
                        for (int s = 0; s < samples; s++) {
                            f32 angle = static_cast<f32>(s) * (2.0f * 3.14159265f / static_cast<f32>(samples));
                            f32 dx = radius * std::cos(angle);
                            f32 dy = radius * std::sin(angle);
                            drawPass(dx, dy, gc);
                        }
                    }
                }
                
                // ---- Pass 2: Shadow ----
                if (textShadowColor().a > 0 &&
                    (textShadowOffsetX() != 0 || textShadowOffsetY() != 0 || textShadowBlurRadius() > 0)) {
                    if (textShadowBlurRadius() > 0) {
                        // Soft shadow: multi-pass ring around offset
                        const int blurSteps = std::max(2, static_cast<int>(textShadowBlurRadius()));
                        const int blurSamples = 8;
                        f32 baseAlpha = static_cast<f32>(textShadowColor().a) / 255.0f;
                        for (int r = 0; r <= blurSteps; r++) {
                            f32 t = static_cast<f32>(r) / static_cast<f32>(blurSteps);
                            f32 radius = t * textShadowBlurRadius();
                            f32 alpha = baseAlpha * (1.0f - t * 0.6f) / static_cast<f32>(blurSteps + 1);
                            Color sc = Color::fromRgba8(textShadowColor().r, textShadowColor().g, textShadowColor().b,
                                                        static_cast<u8>(std::min(255.0f, alpha * 255.0f)));
                            if (r == 0) {
                                drawPass(textShadowOffsetX(), textShadowOffsetY(), sc);
                            } else {
                                for (int s = 0; s < blurSamples; s++) {
                                    f32 angle = static_cast<f32>(s) * (2.0f * 3.14159265f / static_cast<f32>(blurSamples));
                                    drawPass(textShadowOffsetX() + radius * std::cos(angle),
                                             textShadowOffsetY() + radius * std::sin(angle), sc);
                                }
                            }
                        }
                    } else {
                        // Hard shadow: single pass at offset
                        drawPass(textShadowOffsetX(), textShadowOffsetY(), textShadowColor());
                    }
                }
                
                // ---- Pass 3: Outline / Stroke ----
                if (textStrokeColor().a > 0 && textStrokeWidth() > 0) {
                    f32 sw = textStrokeWidth();
                    // 8-direction outline for thin strokes, more samples for thicker
                    int steps = (sw <= 1.5f) ? 8 : 16;
                    for (int s = 0; s < steps; s++) {
                        f32 angle = static_cast<f32>(s) * (2.0f * 3.14159265f / static_cast<f32>(steps));
                        f32 dx = sw * std::cos(angle);
                        f32 dy = sw * std::sin(angle);
                        drawPass(dx, dy, textStrokeColor());
                    }
                }
                
                // ---- Pass 4: Normal foreground text (topmost) ----
                drawPass(0, 0, foreground());
                return;
            }
        }
    }
    
    // Fallback: delegate to backend via TextLayout
    TextLayout layout;
    layout.text = text();
    layout.fontFamily = fontFamily();
    layout.fontSize = fontSize();
    layout.bold = bold();
    layout.italic = italic();
    layout.maxWidth = bounds().width;
    layout.lineHeight = lineHeight();
    ctx.drawText(layout, {padding().left, padding().top}, foreground());
}

} // namespace gut


// --- elements/Button.cpp ---


namespace gut {

Button::Button(String buttonLabel) {
    setlabel(std::move(buttonLabel));
    setfocusable(true);
    setpadding(Thickness{8, 4, 8, 4});
    setcursor(CursorType::Hand);
}

Button::Button(String buttonLabel, std::function<void()> onClick)
    : m_onClick(std::move(onClick))
{
    setlabel(std::move(buttonLabel));
    setfocusable(true);
    setpadding(Thickness{8, 4, 8, 4});
    setcursor(CursorType::Hand);
}

void Button::setContent(Ref<Element> content) {
    m_content = std::move(content);
    invalidateLayout();
}

Size2f Button::measureOverride(Size2f availableSize) {
    const auto& pad = padding();
    Size2f contentAvailable = {
        availableSize.width - pad.horizontalSum() - borderWidth() * 2,
        availableSize.height - pad.verticalSum() - borderWidth() * 2
    };
    
    Size2f contentSize;
    
    if (m_content) {
        m_content->measure(contentAvailable);
        contentSize = m_content->desiredSize();
    } else if (!label().empty()) {
        // Measure label text with font system if available
        bool measured = false;
        if (context()) {
            Font* font = context()->defaultFont();
            if (font) {
                auto face = font->getFace(14.0f);
                if (face) {
                    contentSize.width = face->measureWidth(label());
                    contentSize.height = face->lineHeight();
                    measured = true;
                }
            }
        }
        if (!measured) {
            f32 charWidth = 14.0f * 0.5f;
            contentSize.width = static_cast<f32>(label().length()) * charWidth;
            contentSize.height = 14.0f * 1.2f;
        }
    }
    
    return {
        contentSize.width + pad.horizontalSum() + borderWidth() * 2,
        contentSize.height + pad.verticalSum() + borderWidth() * 2
    };
}

void Button::onRender(RenderContext& ctx) {
    Rectf rect = {0, 0, bounds().width, bounds().height};
    
    // Choose background color based on state
    Color bg = background();
    if (!isEnabled()) {
        bg = disabledBackground();
    } else if (isPressed()) {
        bg = pressedBackground();
    } else if (isHovered()) {
        bg = hoverBackground();
    }
    
    // Draw background
    if (cornerRadius() > 0) {
        ctx.fillRoundedRect(rect, cornerRadius(), bg);
        if (borderWidth() > 0) {
            ctx.strokeRoundedRect(rect, cornerRadius(), borderColor(), borderWidth());
        }
    } else {
        ctx.fillRect(rect, bg);
        if (borderWidth() > 0) {
            ctx.strokeRect(rect, borderColor(), borderWidth());
        }
    }
    
    // Draw content or label
    if (m_content) {
        m_content->render(ctx);
    } else if (!label().empty()) {
        bool drawn = false;
        if (context()) {
            Font* font = context()->defaultFont();
            if (font) {
                auto face = font->getFace(14.0f);
                if (face) {
                    f32 textW = face->measureWidth(label());
                    f32 textH = face->lineHeight();
                    f32 x = (bounds().width - textW) / 2;
                    f32 y = (bounds().height - textH) / 2 + face->ascender();
                    ctx.drawText(face.get(), label(), {x, y}, foreground());
                    drawn = true;
                }
            }
        }
        if (!drawn) {
            TextLayout layout;
            layout.text = label();
            layout.fontSize = 14.0f;
            Size2f textSize = ctx.measureText(layout);
            Point2f textPos = {
                (bounds().width - textSize.width) / 2,
                (bounds().height - textSize.height) / 2
            };
            ctx.drawText(layout, textPos, foreground());
        }
    }
    
    // Draw focus indicator — prominent outer ring
    if (isFocused()) {
        Color fc = focusBorderColor();
        // Outer glow ring (2px outside the button)
        Rectf outerRect = {-3, -3, bounds().width + 6, bounds().height + 6};
        ctx.strokeRoundedRect(outerRect, cornerRadius() + 3, fc, 2.0f);
        // Inner highlight ring
        Rectf innerRect = {1, 1, bounds().width - 2, bounds().height - 2};
        ctx.strokeRoundedRect(innerRect, std::max(cornerRadius() - 1, 0.0f),
                              Color(fc.r, fc.g, fc.b, 0.4f), 1.5f);
    }
}

bool Button::onMouseEvent(const MouseEvent& event) {
    if (!isEnabled()) {
        return false;
    }
    
    switch (event.type) {
        case MouseEventType::ButtonDown:
            if (event.button == MouseButton::Left) {
                setisPressed(true);
                return true;
            }
            break;
            
        case MouseEventType::ButtonUp:
            if (event.button == MouseButton::Left && isPressed()) {
                setisPressed(false);
                if (containsPoint(event.position)) {
                    clicked().emit();
                    if (m_onClick) {
                        m_onClick();
                    }
                }
                return true;
            }
            break;
            
        default:
            break;
    }
    
    return false;
}

void Button::onMouseEnter() {
    Element::onMouseEnter();
    invalidateRender();
}

void Button::onMouseLeave() {
    Element::onMouseLeave();
    setisPressed(false);
    invalidateRender();
}

} // namespace gut


// --- elements/CheckBox.cpp ---

namespace gut {

CheckBox::CheckBox(String text) {
    setlabel(std::move(text));
    setfocusable(true);
    setcursor(CursorType::Hand);
}

void CheckBox::toggle() {
    setisChecked(!isChecked());
    if (m_onCheckedChanged) {
        m_onCheckedChanged(isChecked());
    }
}

Size2f CheckBox::measureOverride(Size2f availableSize) {
    f32 textW = 0;
    f32 textH = boxSize();
    
    if (!label().empty() && context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(fontSize());
            if (face) {
                textW = face->measureWidth(label());
                textH = std::max(textH, face->lineHeight());
            }
        }
    }
    
    f32 totalW = boxSize() + (textW > 0 ? spacing() + textW : 0);
    return {totalW, textH};
}

void CheckBox::onRender(RenderContext& ctx) {
    f32 bs = boxSize();
    f32 cy = (bounds().height - bs) * 0.5f;
    Rectf box = {0, cy, bs, bs};
    
    bool checked = isChecked();
    bool hovered = isHovered();
    bool disabled = !isEnabled();
    
    // Box fill
    Color bg = checked ? checkedBackground() : boxBackground();
    if (disabled) bg = Color::fromRgba8(bg.r * 0.5f, bg.g * 0.5f, bg.b * 0.5f, bg.a);
    ctx.fillRoundedRect(box, boxCornerRadius(), bg);
    
    // Box border
    Color border = checked ? checkedBorderColor()
                  : hovered ? hoverBorderColor()
                  : boxBorderColor();
    ctx.strokeRoundedRect(box, boxCornerRadius(), border, 1.0f);
    
    // Checkmark (two lines forming a check)
    if (checked) {
        Color cm = checkmarkColor();
        f32 t = std::max(1.5f, bs * 0.1f);
        // The check shape: short down-stroke then long up-stroke
        f32 x0 = box.x + bs * 0.22f;
        f32 y0 = box.y + bs * 0.50f;
        f32 x1 = box.x + bs * 0.42f;
        f32 y1 = box.y + bs * 0.72f;
        f32 x2 = box.x + bs * 0.78f;
        f32 y2 = box.y + bs * 0.30f;
        ctx.drawLine({x0, y0}, {x1, y1}, cm, t);
        ctx.drawLine({x1, y1}, {x2, y2}, cm, t);
    }
    
    // Label
    if (!label().empty() && context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(fontSize());
            if (face) {
                Color fg = disabled ? disabledForeground() : foreground();
                f32 textX = bs + spacing();
                f32 textY = (bounds().height - face->lineHeight()) * 0.5f + face->ascender();
                ctx.drawText(face.get(), label(), {textX, textY}, fg);
            }
        }
    }
    
    // Focus ring
    if (isFocused()) {
        Color fc = focusBorderColor();
        Rectf focusRect = {-2, -2, bounds().width + 4, bounds().height + 4};
        ctx.strokeRoundedRect(focusRect, boxCornerRadius() + 2, fc, 1.5f);
    }
}

bool CheckBox::onMouseEvent(const MouseEvent& event) {
    if (!isEnabled()) return false;
    
    switch (event.type) {
        case MouseEventType::ButtonDown:
            if (event.button == MouseButton::Left) {
                setisPressed(true);
                return true;
            }
            break;
        case MouseEventType::ButtonUp:
            if (event.button == MouseButton::Left && isPressed()) {
                setisPressed(false);
                if (containsPoint(event.position)) {
                    toggle();
                    clicked().emit();
                }
                return true;
            }
            break;
        default: break;
    }
    return false;
}

bool CheckBox::onKeyEvent(const KeyEvent& event) {
    if (event.type == KeyEventType::KeyDown && event.key == Key::Space) {
        toggle();
        clicked().emit();
        return true;
    }
    return false;
}

void CheckBox::onMouseEnter() {
    Element::onMouseEnter();
    invalidateRender();
}

void CheckBox::onMouseLeave() {
    Element::onMouseLeave();
    setisPressed(false);
    invalidateRender();
}

} // namespace gut


// --- elements/RadioButton.cpp ---

namespace gut {

RadioButton::RadioButton(String text, String group) {
    setlabel(std::move(text));
    setgroupName(std::move(group));
    setfocusable(true);
    setcursor(CursorType::Hand);
}

void RadioButton::uncheckSiblings() {
    if (!parent()) return;
    Panel* p = dynamic_cast<Panel*>(parent());
    if (!p) return;
    for (usize i = 0; i < p->childCount(); ++i) {
        RadioButton* rb = dynamic_cast<RadioButton*>(p->childAt(i));
        if (rb && rb != this && rb->groupName() == groupName()) {
            if (rb->isChecked()) {
                rb->setisChecked(false);
                if (rb->m_onCheckedChanged) {
                    rb->m_onCheckedChanged(false);
                }
            }
        }
    }
}

void RadioButton::check() {
    if (isChecked()) return;  // already checked — nothing to do
    uncheckSiblings();
    setisChecked(true);
    if (m_onCheckedChanged) {
        m_onCheckedChanged(true);
    }
}

Size2f RadioButton::measureOverride(Size2f availableSize) {
    f32 textW = 0;
    f32 textH = circleSize();
    
    if (!label().empty() && context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(fontSize());
            if (face) {
                textW = face->measureWidth(label());
                textH = std::max(textH, face->lineHeight());
            }
        }
    }
    
    f32 totalW = circleSize() + (textW > 0 ? spacing() + textW : 0);
    return {totalW, textH};
}

void RadioButton::onRender(RenderContext& ctx) {
    f32 cs = circleSize();
    f32 r = cs * 0.5f;
    f32 cy = bounds().height * 0.5f;
    Point2f center = {r, cy};
    
    bool checked = isChecked();
    bool hovered = isHovered();
    bool disabled = !isEnabled();
    
    // Outer circle fill
    Color bg = checked ? checkedBackground() : circleBackground();
    if (disabled) bg = Color::fromRgba8(bg.r * 0.5f, bg.g * 0.5f, bg.b * 0.5f, bg.a);
    ctx.fillEllipse(center, r, r, bg);
    
    // Outer circle border
    Color border = checked ? checkedBorderColor()
                  : hovered ? hoverBorderColor()
                  : circleBorderColor();
    ctx.strokeEllipse(center, r, r, border, 1.0f);
    
    // Inner dot when checked
    if (checked) {
        f32 dotR = r * 0.38f;
        ctx.fillEllipse(center, dotR, dotR, dotColor());
    }
    
    // Label
    if (!label().empty() && context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(fontSize());
            if (face) {
                Color fg = disabled ? disabledForeground() : foreground();
                f32 textX = cs + spacing();
                f32 textY = (bounds().height - face->lineHeight()) * 0.5f + face->ascender();
                ctx.drawText(face.get(), label(), {textX, textY}, fg);
            }
        }
    }
    
    // Focus ring
    if (isFocused()) {
        Color fc = focusBorderColor();
        Rectf focusRect = {-2, -2, bounds().width + 4, bounds().height + 4};
        ctx.strokeRoundedRect(focusRect, cs * 0.5f + 2, fc, 1.5f);
    }
}

bool RadioButton::onMouseEvent(const MouseEvent& event) {
    if (!isEnabled()) return false;
    
    switch (event.type) {
        case MouseEventType::ButtonDown:
            if (event.button == MouseButton::Left) {
                setisPressed(true);
                return true;
            }
            break;
        case MouseEventType::ButtonUp:
            if (event.button == MouseButton::Left && isPressed()) {
                setisPressed(false);
                if (containsPoint(event.position)) {
                    check();
                    clicked().emit();
                }
                return true;
            }
            break;
        default: break;
    }
    return false;
}

bool RadioButton::onKeyEvent(const KeyEvent& event) {
    if (event.type == KeyEventType::KeyDown && event.key == Key::Space) {
        check();
        clicked().emit();
        return true;
    }
    return false;
}

void RadioButton::onMouseEnter() {
    Element::onMouseEnter();
    invalidateRender();
}

void RadioButton::onMouseLeave() {
    Element::onMouseLeave();
    setisPressed(false);
    invalidateRender();
}

} // namespace gut


// --- elements/Toggle.cpp ---

#include <cmath>

namespace gut {

Toggle::Toggle(String text) {
    setlabel(std::move(text));
    setfocusable(true);
    setcursor(CursorType::Hand);
}

void Toggle::doToggle() {
    setisOn(!isOn());
    m_thumbT = isOn() ? 1.0f : 0.0f;
    if (m_onToggled) {
        m_onToggled(isOn());
    }
    invalidateRender();
}

Size2f Toggle::measureOverride(Size2f availableSize) {
    f32 textW = 0;
    f32 textH = trackHeight();

    if (!label().empty() && context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(fontSize());
            if (face) {
                textW = face->measureWidth(label());
                textH = std::max(textH, face->lineHeight());
            }
        }
    }

    f32 totalW = trackWidth() + (textW > 0 ? spacing() + textW : 0);
    return {totalW, textH};
}

void Toggle::onRender(RenderContext& ctx) {
    f32 tw = trackWidth();
    f32 th = trackHeight();
    f32 cr = th * 0.5f;   // pill-shaped: corner radius = half height
    f32 cy = (bounds().height - th) * 0.5f;  // vertical center offset
    Rectf track = {0, cy, tw, th};

    bool on = isOn();
    bool hovered = isHovered();
    bool disabled = !isEnabled();

    // Snap thumb t to state (will be animated later)
    m_thumbT = on ? 1.0f : 0.0f;

    // --- Track fill ---
    Color trackBg;
    if (disabled) {
        Color base = on ? trackOnBackground() : trackOffBackground();
        trackBg = Color::fromRgba8(
            static_cast<u8>(base.r * 0.5f),
            static_cast<u8>(base.g * 0.5f),
            static_cast<u8>(base.b * 0.5f), base.a);
    } else {
        // Lerp between off and on colors based on thumb position
        Color offBg = trackOffBackground();
        Color onBg = trackOnBackground();
        f32 t = m_thumbT;
        trackBg = Color::fromRgba8(
            static_cast<u8>(offBg.r + (onBg.r - offBg.r) * t),
            static_cast<u8>(offBg.g + (onBg.g - offBg.g) * t),
            static_cast<u8>(offBg.b + (onBg.b - offBg.b) * t),
            static_cast<u8>(offBg.a + (onBg.a - offBg.a) * t));
    }
    ctx.fillRoundedRect(track, cr, trackBg);

    // --- Track border ---
    Color border;
    if (isFocused()) {
        border = focusBorderColor();
    } else if (hovered && !disabled) {
        border = hoverBorderColor();
    } else {
        border = on ? trackOnBorderColor() : trackOffBorderColor();
    }
    ctx.strokeRoundedRect(track, cr, border, 1.0f);

    // --- Thumb ---
    f32 tr = thumbRadius();
    f32 tcx = thumbOffX() + m_thumbT * (thumbOnX() - thumbOffX());
    f32 tcy = cy + thumbCenterY();

    // Thumb shadow (slightly below and larger)
    if (!disabled) {
        ctx.fillCircle({tcx, tcy + 1.0f}, tr + 0.5f, thumbShadowColor());
    }

    // Thumb circle
    Color tc = thumbColor();
    if (disabled) {
        tc = Color::fromRgba8(
            static_cast<u8>(tc.r * 0.7f),
            static_cast<u8>(tc.g * 0.7f),
            static_cast<u8>(tc.b * 0.7f), tc.a);
    }
    ctx.fillCircle({tcx, tcy}, tr, tc);

    // --- Label ---
    if (!label().empty() && context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(fontSize());
            if (face) {
                Color fg = disabled ? disabledForeground() : foreground();
                f32 textX = tw + spacing();
                f32 textY = (bounds().height - face->lineHeight()) * 0.5f + face->ascender();
                ctx.drawText(face.get(), label(), {textX, textY}, fg);
            }
        }
    }

    // --- Focus ring ---
    if (isFocused()) {
        Rectf focusRect = {-2, cy - 2, tw + 4, th + 4};
        ctx.strokeRoundedRect(focusRect, cr + 2, focusBorderColor(), 1.5f);
    }
}

bool Toggle::onMouseEvent(const MouseEvent& event) {
    if (!isEnabled()) return false;

    switch (event.type) {
        case MouseEventType::ButtonDown:
            if (event.button == MouseButton::Left) {
                setisPressed(true);
                return true;
            }
            break;
        case MouseEventType::ButtonUp:
            if (event.button == MouseButton::Left && isPressed()) {
                setisPressed(false);
                if (containsPoint(event.position)) {
                    doToggle();
                    clicked().emit();
                }
                return true;
            }
            break;
        default: break;
    }
    return false;
}

bool Toggle::onKeyEvent(const KeyEvent& event) {
    if (event.type == KeyEventType::KeyDown && event.key == Key::Space) {
        doToggle();
        clicked().emit();
        return true;
    }
    return false;
}

void Toggle::onMouseEnter() {
    Element::onMouseEnter();
    invalidateRender();
}

void Toggle::onMouseLeave() {
    Element::onMouseLeave();
    setisPressed(false);
    invalidateRender();
}

} // namespace gut


// --- elements/Slider.cpp ---

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace gut {

Slider::Slider(f32 initialValue) {
    setvalue(initialValue);
    setfocusable(true);
    setcursor(CursorType::Hand);
}

f32 Slider::normalizedValue() const {
    f32 range = maximum() - minimum();
    if (range <= 0.0f) return 0.0f;
    return std::clamp((value() - minimum()) / range, 0.0f, 1.0f);
}

f32 Slider::valueLabelWidth() const {
    if (!showValue()) return 0.0f;
    return labelSpacing() + 40.0f; // space for numeric label
}

void Slider::setValueFromPosition(f32 localX) {
    f32 tl = trackLeft();
    f32 tw = trackWidth();
    if (tw <= 0.0f) return;

    f32 t = std::clamp((localX - tl) / tw, 0.0f, 1.0f);
    f32 newVal = minimum() + t * (maximum() - minimum());

    // Snap to step
    if (step() > 0.0f) {
        newVal = minimum() + std::round((newVal - minimum()) / step()) * step();
        newVal = std::clamp(newVal, minimum(), maximum());
    }

    if (newVal != value()) {
        setvalue(newVal);
        if (m_onValueChanged) m_onValueChanged(value());
        invalidateRender();
    }
}

Size2f Slider::measureOverride(Size2f /*availableSize*/) {
    f32 h = std::max(thumbRadius() * 2.0f, trackHeight());
    f32 w = preferredWidth() + valueLabelWidth();
    return {w, h};
}

void Slider::onRender(RenderContext& ctx) {
    bool disabled = !isEnabled();
    f32 tl = trackLeft();
    f32 tw = trackWidth();
    f32 th = trackHeight();
    f32 cy = bounds().height * 0.5f;
    f32 norm = normalizedValue();
    f32 tr = th * 0.5f; // track corner radius

    // --- Track background ---
    Rectf trackRect = {tl, cy - th * 0.5f, tw, th};
    ctx.fillRoundedRect(trackRect, tr, trackBackground());
    ctx.strokeRoundedRect(trackRect, tr, trackBorderColor(), 0.5f);

    // --- Filled portion ---
    f32 fillW = tw * norm;
    if (fillW > 0.5f) {
        Rectf fillRect = {tl, cy - th * 0.5f, fillW, th};
        Color fill = disabled ? disabledTrackFill() : trackFillColor();
        ctx.fillRoundedRect(fillRect, tr, fill);
    }

    // --- Thumb ---
    f32 thumbX = tl + tw * norm;
    f32 thumbR = thumbRadius();
    Point2f thumbCenter = {thumbX, cy};

    // Shadow
    ctx.fillCircle({thumbX + 0.5f, cy + 1.0f}, thumbR, thumbShadowColor());

    // Thumb body
    Color tc = disabled ? disabledThumbColor()
             : isPressed() ? thumbPressedColor()
             : isHovered() ? thumbHoverColor()
             : thumbColor();
    ctx.fillCircle(thumbCenter, thumbR, tc);

    // Thumb border
    // Use a slightly smaller circle stroke to simulate a border
    ctx.fillCircle(thumbCenter, thumbR, tc);
    // Draw a thin ring around thumb
    {
        // Approximate border: draw a circle outline using a 1px stroke fill trick
        // Since we don't have strokeCircle, draw an outer circle in border color,
        // then the inner fill on top
        Color border = (isHovered() && !disabled) ? focusBorderColor() : thumbBorderColor();
        ctx.fillCircle(thumbCenter, thumbR + 1.0f, border);
        ctx.fillCircle(thumbCenter, thumbR - 0.5f, tc);
    }

    // --- Value label ---
    if (showValue() && context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(fontSize());
            if (face) {
                char buf[32];
                // Show integer if step >= 1, otherwise 1 decimal
                if (step() >= 1.0f) {
                    std::snprintf(buf, sizeof(buf), "%.0f", value());
                } else {
                    std::snprintf(buf, sizeof(buf), "%.1f", value());
                }
                String valStr(buf);
                f32 lx = trackRight() + labelSpacing();
                f32 ly = cy - face->lineHeight() * 0.5f + face->ascender();
                Color fg = disabled ? Color::fromHex(0x808080) : Color::fromHex(0xE0E0E0);
                ctx.drawText(face.get(), valStr, {lx, ly}, fg);
            }
        }
    }

    // --- Focus ring ---
    if (isFocused()) {
        Rectf focusRect = {tl - 3, cy - thumbR - 3, tw + 6, thumbR * 2 + 6};
        ctx.strokeRoundedRect(focusRect, thumbR + 3, focusBorderColor(), 1.5f);
    }
}

bool Slider::onMouseEvent(const MouseEvent& event) {
    if (!isEnabled()) return false;

    switch (event.type) {
        case MouseEventType::ButtonDown:
            if (event.button == MouseButton::Left) {
                m_dragging = true;
                setisPressed(true);
                context()->inputManager().captureMouse(this);
                setValueFromPosition(event.position.x);
                return true;
            }
            break;
        case MouseEventType::ButtonUp:
            if (event.button == MouseButton::Left && m_dragging) {
                m_dragging = false;
                setisPressed(false);
                context()->inputManager().releaseMouse();
                invalidateRender();
                return true;
            }
            break;
        case MouseEventType::Move:
            if (m_dragging) {
                setValueFromPosition(event.position.x);
                return true;
            }
            break;
        default: break;
    }
    return false;
}

bool Slider::onKeyEvent(const KeyEvent& event) {
    if (event.type != KeyEventType::KeyDown) return false;

    f32 s = step() > 0.0f ? step() : (maximum() - minimum()) * 0.01f;

    switch (event.key) {
        case Key::Right:
        case Key::Up: {
            f32 newVal = std::min(value() + s, maximum());
            if (newVal != value()) {
                setvalue(newVal);
                if (m_onValueChanged) m_onValueChanged(value());
                invalidateRender();
            }
            return true;
        }
        case Key::Left:
        case Key::Down: {
            f32 newVal = std::max(value() - s, minimum());
            if (newVal != value()) {
                setvalue(newVal);
                if (m_onValueChanged) m_onValueChanged(value());
                invalidateRender();
            }
            return true;
        }
        case Key::Home:
            if (value() != minimum()) {
                setvalue(minimum());
                if (m_onValueChanged) m_onValueChanged(value());
                invalidateRender();
            }
            return true;
        case Key::End:
            if (value() != maximum()) {
                setvalue(maximum());
                if (m_onValueChanged) m_onValueChanged(value());
                invalidateRender();
            }
            return true;
        default: break;
    }
    return false;
}

void Slider::onMouseEnter() {
    Element::onMouseEnter();
    invalidateRender();
}

void Slider::onMouseLeave() {
    Element::onMouseLeave();
    if (!m_dragging) setisPressed(false);
    invalidateRender();
}

} // namespace gut


// --- elements/ProgressBar.cpp ---

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace gut {

ProgressBar::ProgressBar(f32 initialValue) {
    setvalue(initialValue);
    setisHitTestVisible(false);  // purely visual by default
}

f32 ProgressBar::normalizedValue() const {
    f32 range = maximum() - minimum();
    if (range <= 0.0f) return 0.0f;
    return std::clamp((value() - minimum()) / range, 0.0f, 1.0f);
}

Size2f ProgressBar::measureOverride(Size2f /*availableSize*/) {
    f32 h = barHeight();
    // If showing label below, add space for text
    if (showLabel() && context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(fontSize());
            if (face) {
                h += face->lineHeight() + 4.0f;
            }
        }
    }
    return {preferredWidth(), h};
}

void ProgressBar::onRender(RenderContext& ctx) {
    f32 bw = bounds().width;
    f32 bh = barHeight();
    f32 cr = cornerRadius();
    f32 norm = normalizedValue();
    bool complete = (norm >= 1.0f);

    // --- Track background ---
    Rectf trackRect = {0, 0, bw, bh};
    ctx.fillRoundedRect(trackRect, cr, trackColor());
    ctx.strokeRoundedRect(trackRect, cr, trackBorderColor(), 0.5f);

    // --- Fill ---
    if (!indeterminate()) {
        f32 fillW = bw * norm;
        if (fillW > 1.0f) {
            // Clamp corner radius so it doesn't exceed the fill width
            f32 fcr = std::min(cr, fillW * 0.5f);
            Rectf fillRect = {0, 0, fillW, bh};
            Color fc = complete ? completedFillColor() : fillColor();
            ctx.fillRoundedRect(fillRect, fcr, fc);
        }
    } else {
        // Indeterminate: draw a sliding highlight pulse
        // Use a simple time-based animation via the current value modulo
        f32 pulseW = bw * 0.3f;
        // Animate based on a trick: we read `value()` as a frame counter driven externally,
        // or just draw a static centred bar if no animation driver.
        f32 t = std::fmod(value() * 0.02f, 1.4f) - 0.2f;  // -0.2 .. 1.2
        f32 px = t * bw;
        f32 pw = std::min(pulseW, bw);
        // Clip to bar bounds
        f32 x0 = std::max(0.0f, px);
        f32 x1 = std::min(bw, px + pw);
        if (x1 > x0) {
            Rectf pulseRect = {x0, 0, x1 - x0, bh};
            ctx.fillRoundedRect(pulseRect, std::min(cr, (x1 - x0) * 0.5f), fillColor());
        }
    }

    // --- Percentage text centred in bar ---
    if (showPercentInBar() && !indeterminate() && context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(fontSize());
            if (face) {
                char buf[16];
                std::snprintf(buf, sizeof(buf), "%.0f%%", norm * 100.0f);
                String label(buf);
                f32 tw = face->measureWidth(label);
                f32 tx = (bw - tw) * 0.5f;
                f32 ty = (bh - face->lineHeight()) * 0.5f + face->ascender();
                // Use white or dark text depending on fill coverage
                Color tc = (norm > 0.5f) ? Color::white() : labelColor();
                ctx.drawText(face.get(), label, {tx, ty}, tc);
            }
        }
    }

    // --- Label below bar ---
    if (showLabel() && !showPercentInBar() && context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(fontSize());
            if (face) {
                char buf[16];
                if (indeterminate()) {
                    std::snprintf(buf, sizeof(buf), "Loading...");
                } else {
                    std::snprintf(buf, sizeof(buf), "%.0f%%", norm * 100.0f);
                }
                String label(buf);
                f32 ty = bh + 4.0f + face->ascender();
                ctx.drawText(face.get(), label, {0, ty}, labelColor());
            }
        }
    }
}

} // namespace gut


// --- elements/DropDown.cpp ---

#include <algorithm>
#include <cmath>

namespace gut {

DropDown::DropDown() {
    setfocusable(true);
    setcursor(CursorType::Hand);
}

// ---- Item management --------------------------------------------------------

void DropDown::addItem(String item) {
    m_items.push_back(std::move(item));
    invalidateLayout();
}

void DropDown::removeItem(usize index) {
    if (index < m_items.size()) {
        m_items.erase(m_items.begin() + static_cast<isize>(index));
        if (selectedIndex() >= static_cast<isize>(m_items.size())) {
            setselectedIndex(static_cast<isize>(m_items.size()) - 1);
        }
        invalidateLayout();
    }
}

void DropDown::clearItems() {
    m_items.clear();
    setselectedIndex(-1);
    m_scrollOffset = 0;
    invalidateLayout();
}

String DropDown::selectedItem() const {
    isize idx = selectedIndex();
    if (idx >= 0 && idx < static_cast<isize>(m_items.size())) {
        return m_items[static_cast<usize>(idx)];
    }
    return {};
}

// ---- Open / close -----------------------------------------------------------

void DropDown::open() {
    if (m_isOpen || m_items.empty()) return;
    m_isOpen = true;

    // Decide direction: open downward unless there's not enough space below
    Rectf sb = screenBounds();
    Size2f winSz = context()->size();
    f32 spaceBelow = winSz.height - (sb.y + sb.height);
    f32 spaceAbove = sb.y;

    m_popupHeight = effectivePopupHeight();
    m_opensUpward = (spaceBelow < m_popupHeight && spaceAbove > spaceBelow);

    // Ensure the selected item is visible
    if (selectedIndex() >= 0) {
        f32 itemTop = static_cast<f32>(selectedIndex()) * itemHeight();
        f32 itemBot = itemTop + itemHeight();
        if (itemTop < m_scrollOffset) {
            m_scrollOffset = itemTop;
        } else if (itemBot > m_scrollOffset + m_popupHeight) {
            m_scrollOffset = itemBot - m_popupHeight;
        }
    }
    m_hoveredItemIndex = selectedIndex();

    // Capture the mouse so we get all clicks (for dismiss-on-click-outside)
    if (context()) {
        context()->inputManager().captureMouse(this);

        // Register an overlay so the popup renders on top of everything
        context()->addOverlay(this, [this](RenderContext& ctx) {
            renderPopupOverlay(ctx);
        });
    }
    invalidateRender();
}

void DropDown::close() {
    if (!m_isOpen) return;
    m_isOpen = false;
    m_hoveredItemIndex = -1;
    if (context()) {
        context()->inputManager().releaseMouse();
        context()->removeOverlay(this);
    }
    invalidateRender();
}

void DropDown::toggle() {
    if (m_isOpen) close(); else open();
}

// ---- Helpers ----------------------------------------------------------------

f32 DropDown::totalItemsHeight() const {
    return static_cast<f32>(m_items.size()) * itemHeight();
}

f32 DropDown::effectivePopupHeight() const {
    return std::min(totalItemsHeight(), maxDropHeight());
}

bool DropDown::needsScroll() const {
    return totalItemsHeight() > m_popupHeight;
}

f32 DropDown::maxScrollOffset() const {
    return std::max(0.0f, totalItemsHeight() - m_popupHeight);
}

Rectf DropDown::popupLocalRect() const {
    f32 bh = bounds().height;
    if (m_opensUpward) {
        return {0, -m_popupHeight - 2.0f, bounds().width, m_popupHeight};
    } else {
        return {0, bh + 2.0f, bounds().width, m_popupHeight};
    }
}

isize DropDown::itemIndexAtLocalY(f32 localY) const {
    Rectf pr = popupLocalRect();
    f32 relY = localY - pr.y + m_scrollOffset;
    if (relY < 0) return -1;
    isize idx = static_cast<isize>(relY / itemHeight());
    if (idx >= static_cast<isize>(m_items.size())) return -1;
    return idx;
}

// ---- Measure ----------------------------------------------------------------

Size2f DropDown::measureOverride(Size2f availableSize) {
    // Button height is itemHeight, width stretches to available or content
    f32 w = 0;
    if (context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(fontSize());
            if (face) {
                // Measure widest item
                for (const auto& item : m_items) {
                    w = std::max(w, face->measureWidth(item));
                }
                // Also measure placeholder
                w = std::max(w, face->measureWidth(placeholder()));
            }
        }
    }
    // Add padding + arrow space
    w += kPadH * 2 + arrowSize() + kPadH;
    return {w, itemHeight()};
}

// ---- Render -----------------------------------------------------------------

void DropDown::onRender(RenderContext& ctx) {
    f32 bw = bounds().width;
    f32 bh = bounds().height;
    f32 cr = cornerRadius();
    bool disabled = !isEnabled();
    bool hovered = isHovered();

    // --- Button background ---
    ctx.fillRoundedRect({0, 0, bw, bh}, cr, buttonBackground());

    // --- Button border ---
    Color bc = m_isOpen ? focusBorderColor()
             : isFocused() ? focusBorderColor()
             : hovered ? hoverBorderColor()
             : borderColor();
    ctx.strokeRoundedRect({0, 0, bw, bh}, cr, bc, 1.0f);

    // --- Selected text or placeholder ---
    if (context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(fontSize());
            if (face) {
                String text = selectedItem();
                Color fg = disabled ? disabledForeground() : foreground();
                if (text.empty()) {
                    text = placeholder();
                    fg = Color::fromRgba8(fg.r * 0.5f, fg.g * 0.5f, fg.b * 0.5f, fg.a);
                }
                f32 textY = (bh - face->lineHeight()) * 0.5f + face->ascender();
                // Clip text to not overlap arrow
                ctx.save();
                ctx.pushClip({kPadH, 0, bw - kPadH * 2 - arrowSize() - kPadH, bh});
                ctx.drawText(face.get(), text, {kPadH, textY}, fg);
                ctx.popClip();
                ctx.restore();
            }
        }
    }

    // --- Arrow (chevron) ---
    {
        f32 as = arrowSize();
        f32 ax = bw - kPadH - as;
        f32 ay = (bh - as * 0.5f) * 0.5f;
        Color ac = disabled ? disabledForeground() : arrowColor();
        if (m_isOpen) {
            // Up chevron
            ctx.drawLine({ax, ay + as * 0.5f}, {ax + as * 0.5f, ay}, ac, 1.5f);
            ctx.drawLine({ax + as * 0.5f, ay}, {ax + as, ay + as * 0.5f}, ac, 1.5f);
        } else {
            // Down chevron
            ctx.drawLine({ax, ay}, {ax + as * 0.5f, ay + as * 0.5f}, ac, 1.5f);
            ctx.drawLine({ax + as * 0.5f, ay + as * 0.5f}, {ax + as, ay}, ac, 1.5f);
        }
    }

    // Popup is drawn via the overlay system (see renderPopupOverlay)
}

// ---- Popup overlay (rendered on top of everything) --------------------------

void DropDown::renderPopupOverlay(RenderContext& ctx) {
    if (!m_isOpen || !context()) return;

    // The overlay is called in root (identity) coordinate space.
    // Translate so that (0,0) corresponds to our element's screen position.
    Rectf sb = screenBounds();
    ctx.save();
    ctx.translate(sb.x, sb.y);

    f32 cr = cornerRadius();
    Rectf pr = popupLocalRect();
    bool scroll = needsScroll();
    f32 itemW = scroll ? pr.width - kScrollbarWidth : pr.width;

    // Popup shadow
    ctx.drawDropShadow(pr, cr, Color::fromRgba8(0, 0, 0, 100), 12.0f, 0.0f, 4.0f);

    // Popup background
    ctx.fillRoundedRect(pr, cr, dropBackground());

    // Clip items to popup rect
    ctx.save();
    ctx.pushClip(pr, cr);

    // Draw items
    Font* font = context()->defaultFont();
    Ref<FontFace> face;
    if (font) face = font->getFace(fontSize());

    for (isize i = 0; i < static_cast<isize>(m_items.size()); ++i) {
        f32 iy = pr.y + static_cast<f32>(i) * itemHeight() - m_scrollOffset;
        // Skip if out of view
        if (iy + itemHeight() < pr.y || iy > pr.y + pr.height) continue;

        Rectf itemRect = {pr.x, iy, itemW, itemHeight()};

        // Highlight
        if (i == m_hoveredItemIndex) {
            ctx.fillRect(itemRect, itemHoverBackground());
        }
        if (i == selectedIndex()) {
            ctx.fillRect(itemRect, selectedItemBackground());
        }

        // Item text
        if (face) {
            f32 textY = iy + (itemHeight() - face->lineHeight()) * 0.5f + face->ascender();
            ctx.drawText(face.get(), m_items[static_cast<usize>(i)], {pr.x + kPadH, textY}, foreground());
        }
    }

    ctx.popClip();
    ctx.restore();

    // Popup border (on top)
    ctx.strokeRoundedRect(pr, cr, dropBorderColor(), 1.0f);

    // Scrollbar
    if (scroll) {
        f32 sbX = pr.x + pr.width - kScrollbarWidth;
        f32 sbY = pr.y;
        f32 sbH = pr.height;

        // Track
        ctx.fillRoundedRect({sbX, sbY, kScrollbarWidth, sbH}, kScrollbarWidth * 0.5f, scrollbarTrackColor());

        // Thumb
        f32 viewRatio = m_popupHeight / totalItemsHeight();
        f32 thumbH = std::max(20.0f, sbH * viewRatio);
        f32 scrollRatio = (maxScrollOffset() > 0) ? m_scrollOffset / maxScrollOffset() : 0.0f;
        f32 thumbY = sbY + scrollRatio * (sbH - thumbH);
        ctx.fillRoundedRect({sbX, thumbY, kScrollbarWidth, thumbH}, kScrollbarWidth * 0.5f, scrollbarThumbColor());
    }

    ctx.restore();
}

// ---- Mouse events -----------------------------------------------------------

bool DropDown::onMouseEvent(const MouseEvent& event) {
    if (!isEnabled()) return false;

    switch (event.type) {
        case MouseEventType::ButtonDown: {
            if (event.button != MouseButton::Left) break;

            if (m_isOpen) {
                // Check if click is inside popup
                Rectf pr = popupLocalRect();
                if (event.position.x >= pr.x && event.position.x < pr.x + pr.width &&
                    event.position.y >= pr.y && event.position.y < pr.y + pr.height) {
                    // Click on item — select it
                    isize idx = itemIndexAtLocalY(event.position.y);
                    if (idx >= 0) {
                        setselectedIndex(idx);
                        if (m_onSelectionChanged) m_onSelectionChanged(idx);
                        close();
                    }
                } else if (event.position.x >= 0 && event.position.x < bounds().width &&
                           event.position.y >= 0 && event.position.y < bounds().height) {
                    // Click on the button itself — toggle
                    close();
                } else {
                    // Click outside — dismiss
                    close();
                }
                return true;
            } else {
                // Open
                open();
                return true;
            }
        }

        case MouseEventType::Move: {
            if (m_isOpen) {
                Rectf pr = popupLocalRect();
                if (event.position.x >= pr.x && event.position.x < pr.x + pr.width &&
                    event.position.y >= pr.y && event.position.y < pr.y + pr.height) {
                    isize idx = itemIndexAtLocalY(event.position.y);
                    if (idx != m_hoveredItemIndex) {
                        m_hoveredItemIndex = idx;
                        invalidateRender();
                    }
                } else {
                    if (m_hoveredItemIndex != -1) {
                        m_hoveredItemIndex = -1;
                        invalidateRender();
                    }
                }
                return true;
            }
            break;
        }

        case MouseEventType::Wheel: {
            if (m_isOpen && needsScroll()) {
                m_scrollOffset -= event.delta.y * 3.0f;
                m_scrollOffset = std::clamp(m_scrollOffset, 0.0f, maxScrollOffset());
                invalidateRender();
                return true;
            }
            break;
        }

        default:
            break;
    }
    return false;
}

// ---- Keyboard events --------------------------------------------------------

bool DropDown::onKeyEvent(const KeyEvent& event) {
    if (!isEnabled()) return false;
    if (event.type != KeyEventType::KeyDown) return false;

    switch (event.key) {
        case Key::Space:
        case Key::Return:
            if (m_isOpen) {
                // Select hovered item
                if (m_hoveredItemIndex >= 0 && m_hoveredItemIndex < static_cast<isize>(m_items.size())) {
                    setselectedIndex(m_hoveredItemIndex);
                    if (m_onSelectionChanged) m_onSelectionChanged(m_hoveredItemIndex);
                }
                close();
            } else {
                open();
            }
            return true;

        case Key::Escape:
            if (m_isOpen) {
                close();
                return true;
            }
            break;

        case Key::Down:
            if (m_isOpen) {
                if (m_hoveredItemIndex < static_cast<isize>(m_items.size()) - 1) {
                    m_hoveredItemIndex++;
                    // Scroll to keep visible
                    f32 itemBot = static_cast<f32>(m_hoveredItemIndex + 1) * itemHeight();
                    if (itemBot > m_scrollOffset + m_popupHeight) {
                        m_scrollOffset = itemBot - m_popupHeight;
                    }
                    invalidateRender();
                }
            } else {
                // Without opening, cycle selection down
                if (selectedIndex() < static_cast<isize>(m_items.size()) - 1) {
                    setselectedIndex(selectedIndex() + 1);
                    if (m_onSelectionChanged) m_onSelectionChanged(selectedIndex());
                    invalidateRender();
                }
            }
            return true;

        case Key::Up:
            if (m_isOpen) {
                if (m_hoveredItemIndex > 0) {
                    m_hoveredItemIndex--;
                    f32 itemTop = static_cast<f32>(m_hoveredItemIndex) * itemHeight();
                    if (itemTop < m_scrollOffset) {
                        m_scrollOffset = itemTop;
                    }
                    invalidateRender();
                }
            } else {
                if (selectedIndex() > 0) {
                    setselectedIndex(selectedIndex() - 1);
                    if (m_onSelectionChanged) m_onSelectionChanged(selectedIndex());
                    invalidateRender();
                }
            }
            return true;

        case Key::Home:
            if (m_isOpen && !m_items.empty()) {
                m_hoveredItemIndex = 0;
                m_scrollOffset = 0;
                invalidateRender();
                return true;
            }
            break;

        case Key::End:
            if (m_isOpen && !m_items.empty()) {
                m_hoveredItemIndex = static_cast<isize>(m_items.size()) - 1;
                m_scrollOffset = maxScrollOffset();
                invalidateRender();
                return true;
            }
            break;

        default:
            break;
    }
    return false;
}

void DropDown::onMouseEnter() {
    Element::onMouseEnter();
    invalidateRender();
}

void DropDown::onMouseLeave() {
    Element::onMouseLeave();
    invalidateRender();
}

} // namespace gut


// --- elements/TabControl.cpp ---

#include <algorithm>
#include <cmath>

namespace gut {

TabControl::TabControl() {
    setfocusable(true);
    setclipToBounds(true);
}

// ---- Tab management ---------------------------------------------------------

void TabControl::addTab(String title, Ref<Element> content) {
    // Hide content unless this will be the selected tab
    bool isSelected = static_cast<isize>(m_tabs.size()) == selectedIndex();
    content->setvisibility(isSelected ? Visibility::Visible : Visibility::Collapsed);
    Panel::addChild(content);
    m_tabs.push_back({std::move(title), std::move(content)});
    invalidateLayout();
}

void TabControl::removeTab(usize index) {
    if (index >= m_tabs.size()) return;
    Panel::removeChild(m_tabs[index].content.get());
    m_tabs.erase(m_tabs.begin() + static_cast<isize>(index));
    if (selectedIndex() >= static_cast<isize>(m_tabs.size())) {
        selectTab(static_cast<isize>(m_tabs.size()) - 1);
    }
    invalidateLayout();
}

void TabControl::selectTab(isize index) {
    if (index < 0 || index >= static_cast<isize>(m_tabs.size())) return;
    if (index == selectedIndex()) return;

    // Hide old
    isize old = selectedIndex();
    if (old >= 0 && old < static_cast<isize>(m_tabs.size())) {
        m_tabs[static_cast<usize>(old)].content->setvisibility(Visibility::Collapsed);
    }

    setselectedIndex(index);

    // Show new
    m_tabs[static_cast<usize>(index)].content->setvisibility(Visibility::Visible);

    ensureTabVisible(index);

    if (m_onTabChanged) m_onTabChanged(index);
    invalidateLayout();
    invalidateRender();
}

// ---- Helpers ----------------------------------------------------------------

f32 TabControl::tabHeaderWidth(const String& title) const {
    f32 w = tabPadding() * 2;
    if (context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(tabFontSize());
            if (face) {
                w += face->measureWidth(title);
            }
        }
    }
    return w;
}

f32 TabControl::totalTabsWidth() const {
    f32 total = tabSpacing();
    for (const auto& tab : m_tabs) {
        total += tabHeaderWidth(tab.title) + tabSpacing();
    }
    return total;
}

bool TabControl::needsScroll() const {
    return totalTabsWidth() > bounds().width;
}

f32 TabControl::scrollableRegionWidth() const {
    f32 bw = bounds().width;
    if (needsScroll()) {
        return bw - scrollArrowWidth() * 2.0f;  // both arrow buttons
    }
    return bw;
}

void TabControl::ensureTabVisible(isize index) {
    if (index < 0 || index >= static_cast<isize>(m_tabs.size())) return;
    if (!needsScroll()) { m_tabScrollOffset = 0; return; }

    // Compute the left and right edge of the target tab
    f32 left = tabSpacing();
    for (isize i = 0; i < index; ++i) {
        left += tabHeaderWidth(m_tabs[static_cast<usize>(i)].title) + tabSpacing();
    }
    f32 right = left + tabHeaderWidth(m_tabs[static_cast<usize>(index)].title);

    f32 visW = scrollableRegionWidth();
    if (left - m_tabScrollOffset < 0) {
        m_tabScrollOffset = left - tabSpacing();
    } else if (right - m_tabScrollOffset > visW) {
        m_tabScrollOffset = right - visW + tabSpacing();
    }
    // Clamp
    f32 maxOff = totalTabsWidth() - visW;
    if (m_tabScrollOffset > maxOff) m_tabScrollOffset = maxOff;
    if (m_tabScrollOffset < 0) m_tabScrollOffset = 0;
}

isize TabControl::tabIndexAtX(f32 x) const {
    bool scrolling = needsScroll();
    f32 leftArrowW = scrolling ? scrollArrowWidth() : 0.0f;
    f32 rightEdge = scrolling ? bounds().width - scrollArrowWidth() : bounds().width;

    // Clicks on arrow regions don't hit tabs
    if (scrolling && (x < leftArrowW || x >= rightEdge)) return -1;

    f32 cx = tabSpacing() - m_tabScrollOffset + leftArrowW;
    for (isize i = 0; i < static_cast<isize>(m_tabs.size()); ++i) {
        f32 tw = tabHeaderWidth(m_tabs[static_cast<usize>(i)].title);
        f32 tabLeft = cx;
        f32 tabRight = cx + tw;
        // Only consider visible region
        if (tabRight > leftArrowW && tabLeft < rightEdge) {
            if (x >= tabLeft && x < tabRight) return i;
        }
        cx += tw + tabSpacing();
    }
    return -1;
}

// ---- Layout -----------------------------------------------------------------

Size2f TabControl::measureOverride(Size2f availableSize) {
    f32 barH = tabBarHeight();
    Size2f contentAvail = {availableSize.width, availableSize.height - barH};

    // Measure all content pages (even hidden ones for proper sizing)
    Size2f maxContent = {0, 0};
    for (auto& tab : m_tabs) {
        auto oldVis = tab.content->visibility();
        tab.content->setvisibility(Visibility::Visible);
        tab.content->measure(contentAvail);
        Size2f ds = tab.content->desiredSize();
        maxContent.width = std::max(maxContent.width, ds.width);
        maxContent.height = std::max(maxContent.height, ds.height);
        tab.content->setvisibility(oldVis);
    }

    return {maxContent.width, barH + maxContent.height};
}

Size2f TabControl::arrangeOverride(Size2f finalSize) {
    f32 barH = tabBarHeight();
    Rectf contentRect = {0, barH, finalSize.width, finalSize.height - barH};

    for (auto& tab : m_tabs) {
        tab.content->arrange(contentRect);
    }
    return finalSize;
}

// ---- Render -----------------------------------------------------------------

void TabControl::onRender(RenderContext& ctx) {
    f32 bw = bounds().width;
    f32 barH = tabBarHeight();
    bool scrolling = needsScroll();
    f32 leftArrowW = scrolling ? scrollArrowWidth() : 0.0f;
    f32 rightArrowW = scrolling ? scrollArrowWidth() : 0.0f;
    f32 visibleStart = leftArrowW;
    f32 visibleEnd = bw - rightArrowW;

    // Tab bar background
    ctx.fillRect({0, 0, bw, barH}, tabBarBackground());

    // Tab headers
    Font* font = context() ? context()->defaultFont() : nullptr;
    Ref<FontFace> face;
    if (font) face = font->getFace(tabFontSize());

    f32 cx = tabSpacing() - m_tabScrollOffset + leftArrowW;
    for (isize i = 0; i < static_cast<isize>(m_tabs.size()); ++i) {
        const auto& tab = m_tabs[static_cast<usize>(i)];
        f32 tw = tabHeaderWidth(tab.title);
        bool active = (i == selectedIndex());
        bool hovered = (i == m_hoveredTab);

        // Skip tabs that are completely outside the visible region
        if (cx + tw < visibleStart || cx > visibleEnd) {
            cx += tw + tabSpacing();
            continue;
        }

        // Clip to visible region
        f32 drawLeft = std::max(cx, visibleStart);
        f32 drawRight = std::min(cx + tw, visibleEnd);
        f32 drawW = drawRight - drawLeft;
        if (drawW <= 0) { cx += tw + tabSpacing(); continue; }

        // Tab background on hover
        if (hovered && !active) {
            ctx.fillRect({drawLeft, 0, drawW, barH}, tabHoverBackground());
        }

        // Tab text (only if the tab is substantially visible)
        if (face && drawW > 10.0f) {
            Color fg = active ? tabActiveForeground() : tabForeground();
            f32 textW = face->measureWidth(tab.title);
            f32 textX = cx + (tw - textW) * 0.5f;
            f32 textY = (barH - face->lineHeight()) * 0.5f + face->ascender();

            // Clip text drawing
            ctx.save();
            ctx.pushClip({drawLeft, 0, drawW, barH});
            ctx.drawText(face.get(), tab.title, {textX, textY}, fg);
            ctx.popClip();
            ctx.restore();
        }

        // Active indicator (bottom bar)
        if (active) {
            ctx.fillRect({drawLeft, barH - 2.0f, drawW, 2.0f}, tabActiveIndicator());
        }

        cx += tw + tabSpacing();
    }

    // --- Scroll arrow buttons ---
    if (scrolling) {
        bool canScrollLeft = (m_tabScrollOffset > 0.5f);
        bool canScrollRight = (m_tabScrollOffset < totalTabsWidth() - scrollableRegionWidth() - 0.5f);

        // Left arrow button background
        {
            Color bg = m_leftArrowHovered ? tabHoverBackground() : tabBarBackground();
            ctx.fillRect({0, 0, leftArrowW, barH}, bg);
            // Draw left chevron (‹)
            Color fg = canScrollLeft ? tabActiveForeground() : Color::fromRgba8(80, 80, 100, 255);
            if (face) {
                f32 textY = (barH - face->lineHeight()) * 0.5f + face->ascender();
                f32 chevW = face->measureWidth("<");
                ctx.drawText(face.get(), "<", {(leftArrowW - chevW) * 0.5f, textY}, fg);
            }
            // Separator
            ctx.fillRect({leftArrowW - 1.0f, 2.0f, 1.0f, barH - 4.0f}, tabBarBorderColor());
        }

        // Right arrow button background
        {
            f32 rx = bw - rightArrowW;
            Color bg = m_rightArrowHovered ? tabHoverBackground() : tabBarBackground();
            ctx.fillRect({rx, 0, rightArrowW, barH}, bg);
            // Draw right chevron (›)
            Color fg = canScrollRight ? tabActiveForeground() : Color::fromRgba8(80, 80, 100, 255);
            if (face) {
                f32 textY = (barH - face->lineHeight()) * 0.5f + face->ascender();
                f32 chevW = face->measureWidth(">");
                ctx.drawText(face.get(), ">", {rx + (rightArrowW - chevW) * 0.5f, textY}, fg);
            }
            // Separator
            ctx.fillRect({rx, 2.0f, 1.0f, barH - 4.0f}, tabBarBorderColor());
        }
    }

    // Tab bar bottom border
    ctx.fillRect({0, barH - 1.0f, bw, 1.0f}, tabBarBorderColor());

    // Content area background
    if (contentBackground().a > 0) {
        ctx.fillRect({0, barH, bw, bounds().height - barH}, contentBackground());
    }

    // Render children (the visible content page)
    renderChildren(ctx);
}

// ---- Mouse ------------------------------------------------------------------

bool TabControl::onMouseEvent(const MouseEvent& event) {
    f32 barH = tabBarHeight();
    bool scrolling = needsScroll();
    f32 leftArrowW = scrolling ? scrollArrowWidth() : 0.0f;
    f32 rightArrowX = bounds().width - (scrolling ? scrollArrowWidth() : 0.0f);

    switch (event.type) {
        case MouseEventType::ButtonDown: {
            if (event.button == MouseButton::Left && event.position.y < barH) {
                // Check arrow buttons
                if (scrolling && event.position.x < leftArrowW) {
                    // Scroll left
                    f32 step = 80.0f;
                    m_tabScrollOffset = std::max(0.0f, m_tabScrollOffset - step);
                    invalidateRender();
                    return true;
                }
                if (scrolling && event.position.x >= rightArrowX) {
                    // Scroll right
                    f32 maxOff = totalTabsWidth() - scrollableRegionWidth();
                    f32 step = 80.0f;
                    m_tabScrollOffset = std::min(maxOff, m_tabScrollOffset + step);
                    invalidateRender();
                    return true;
                }

                isize idx = tabIndexAtX(event.position.x);
                if (idx >= 0) {
                    selectTab(idx);
                    return true;
                }
            }
            break;
        }
        case MouseEventType::Move: {
            if (event.position.y < barH) {
                // Track arrow hover
                bool leftHov = scrolling && event.position.x < leftArrowW;
                bool rightHov = scrolling && event.position.x >= rightArrowX;
                if (leftHov != m_leftArrowHovered || rightHov != m_rightArrowHovered) {
                    m_leftArrowHovered = leftHov;
                    m_rightArrowHovered = rightHov;
                    invalidateRender();
                }

                isize idx = tabIndexAtX(event.position.x);
                if (idx != m_hoveredTab) {
                    m_hoveredTab = idx;
                    invalidateRender();
                }
            } else {
                if (m_hoveredTab != -1 || m_leftArrowHovered || m_rightArrowHovered) {
                    m_hoveredTab = -1;
                    m_leftArrowHovered = false;
                    m_rightArrowHovered = false;
                    invalidateRender();
                }
            }
            break;
        }
        default:
            break;
    }
    return Panel::onMouseEvent(event);
}

// ---- Keyboard ---------------------------------------------------------------

bool TabControl::onKeyEvent(const KeyEvent& event) {
    if (event.type != KeyEventType::KeyDown) return false;

    switch (event.key) {
        case Key::Left:
            if (selectedIndex() > 0) {
                selectTab(selectedIndex() - 1);
                return true;
            }
            break;
        case Key::Right:
            if (selectedIndex() < static_cast<isize>(m_tabs.size()) - 1) {
                selectTab(selectedIndex() + 1);
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

} // namespace gut


// --- elements/Table.cpp ---

#include <algorithm>
#include <cstdio>

namespace gut {

Table::Table() {
    setfocusable(true);
    setcursor(CursorType::Arrow);
}

// ---- Columns / Rows ---------------------------------------------------------

void Table::addColumn(String title, f32 width) {
    m_columns.push_back({std::move(title), width});
    invalidateLayout();
    invalidateRender();
}

void Table::clearColumns() {
    m_columns.clear();
    invalidateLayout();
    invalidateRender();
}

void Table::setRows(std::vector<Row> rows) {
    m_rows = std::move(rows);
    m_scrollOffset = std::clamp(m_scrollOffset, 0.0f, maxScrollOffset());
    invalidateRender();
}

void Table::clearRows() {
    m_rows.clear();
    setselectedRow(-1);
    m_scrollOffset = 0.0f;
    invalidateRender();
}

void Table::selectRow(isize index) {
    if (index < -1 || index >= static_cast<isize>(m_rows.size())) return;
    setselectedRow(index);
    if (m_onRowSelected) m_onRowSelected(index);
    invalidateRender();
}

// ---- Geometry helpers -------------------------------------------------------

f32 Table::totalColumnsWidth() const {
    f32 w = 0;
    for (auto& col : m_columns) w += col.width;
    return w;
}

isize Table::rowIndexAtY(f32 localY) const {
    f32 bodyTop = headerHeight();
    if (localY < bodyTop) return -1;
    f32 yInBody = localY - bodyTop + m_scrollOffset;
    isize idx = static_cast<isize>(yInBody / rowHeight());
    if (idx < 0 || idx >= static_cast<isize>(m_rows.size())) return -1;
    return idx;
}

isize Table::columnIndexAtX(f32 localX) const {
    f32 cx = 0;
    for (usize i = 0; i < m_columns.size(); ++i) {
        cx += m_columns[i].width;
        if (localX < cx) return static_cast<isize>(i);
    }
    return -1;
}

// ---- Layout -----------------------------------------------------------------

Size2f Table::measureOverride(Size2f availableSize) {
    f32 w = (width() == width()) ? width() : totalColumnsWidth() + (needsScrollbar() ? kScrollbarWidth : 0);
    f32 h = (height() == height()) ? height() : headerHeight() + totalRowsHeight();
    return {w, h};
}

// ---- Render -----------------------------------------------------------------

void Table::onRender(RenderContext& ctx) {
    f32 bw = bounds().width;
    f32 bh = bounds().height;
    f32 hdrH = headerHeight();
    f32 bodyH = bh - hdrH;
    bool scroll = needsScrollbar();
    f32 contentW = scroll ? bw - kScrollbarWidth : bw;

    Font* font = context() ? context()->defaultFont() : nullptr;
    Ref<FontFace> hdrFace, cellFace;
    if (font) {
        hdrFace = font->getFace(headerFontSize());
        cellFace = font->getFace(fontSize());
    }

    // ---- Header ----
    ctx.fillRect({0, 0, bw, hdrH}, headerBackground());

    {
        f32 cx = 0;
        for (usize ci = 0; ci < m_columns.size(); ++ci) {
            const auto& col = m_columns[ci];
            f32 cw = col.width;

            // Hover
            if (static_cast<isize>(ci) == m_hoveredHeaderCol) {
                ctx.fillRect({cx, 0, cw, hdrH}, headerHoverBackground());
            }

            // Header text
            if (hdrFace) {
                f32 textY = (hdrH - hdrFace->lineHeight()) * 0.5f + hdrFace->ascender();
                ctx.save();
                ctx.pushClip({cx, 0, cw - cellPaddingH(), hdrH});
                ctx.drawText(hdrFace.get(), col.title, {cx + cellPaddingH(), textY}, headerForeground());
                ctx.popClip();
                ctx.restore();
            }

            // Sort indicator
            if (sortColumn() == static_cast<isize>(ci)) {
                f32 arrowX = cx + cw - cellPaddingH() - kSortArrowSize;
                f32 arrowCY = hdrH * 0.5f;
                if (sortAscending()) {
                    // Up arrow  ▲
                    ctx.drawLine({arrowX, arrowCY + kSortArrowSize * 0.35f},
                                 {arrowX + kSortArrowSize * 0.5f, arrowCY - kSortArrowSize * 0.35f},
                                 sortArrowColor(), 1.5f);
                    ctx.drawLine({arrowX + kSortArrowSize * 0.5f, arrowCY - kSortArrowSize * 0.35f},
                                 {arrowX + kSortArrowSize, arrowCY + kSortArrowSize * 0.35f},
                                 sortArrowColor(), 1.5f);
                } else {
                    // Down arrow  ▼
                    ctx.drawLine({arrowX, arrowCY - kSortArrowSize * 0.35f},
                                 {arrowX + kSortArrowSize * 0.5f, arrowCY + kSortArrowSize * 0.35f},
                                 sortArrowColor(), 1.5f);
                    ctx.drawLine({arrowX + kSortArrowSize * 0.5f, arrowCY + kSortArrowSize * 0.35f},
                                 {arrowX + kSortArrowSize, arrowCY - kSortArrowSize * 0.35f},
                                 sortArrowColor(), 1.5f);
                }
            }

            // Column separator
            if (ci + 1 < m_columns.size()) {
                ctx.drawLine({cx + cw, 2.0f}, {cx + cw, hdrH - 2.0f}, headerBorderColor(), 1.0f);
            }

            cx += cw;
        }
    }

    // Header bottom border
    ctx.fillRect({0, hdrH - 1.0f, bw, 1.0f}, headerBorderColor());

    // ---- Body (clipped) ----
    ctx.save();
    ctx.pushClip({0, hdrH, contentW, bodyH});

    f32 rh = rowHeight();
    isize firstVisible = static_cast<isize>(m_scrollOffset / rh);
    isize lastVisible = static_cast<isize>((m_scrollOffset + bodyH) / rh);
    lastVisible = std::min(lastVisible, static_cast<isize>(m_rows.size()) - 1);

    for (isize ri = firstVisible; ri <= lastVisible; ++ri) {
        f32 ry = hdrH + static_cast<f32>(ri) * rh - m_scrollOffset;

        // Row background — alternating + hover + selection
        Color bg = (ri % 2 == 0) ? rowBackground() : rowAlternateBackground();
        if (ri == m_hoveredRow) bg = rowHoverBackground();
        if (ri == selectedRow()) bg = rowSelectedBackground();
        ctx.fillRect({0, ry, contentW, rh}, bg);

        // Cells
        if (cellFace && ri < static_cast<isize>(m_rows.size())) {
            const auto& row = m_rows[static_cast<usize>(ri)];
            f32 cx = 0;
            for (usize ci = 0; ci < m_columns.size(); ++ci) {
                f32 cw = m_columns[ci].width;
                if (ci < row.size()) {
                    f32 textY = ry + (rh - cellFace->lineHeight()) * 0.5f + cellFace->ascender();
                    ctx.save();
                    ctx.pushClip({cx, ry, cw - 2.0f, rh});
                    ctx.drawText(cellFace.get(), row[ci], {cx + cellPaddingH(), textY}, cellForeground());
                    ctx.popClip();
                    ctx.restore();
                }
                cx += cw;
            }
        }

        // Grid line below row
        ctx.fillRect({0, ry + rh - 1.0f, contentW, 1.0f}, gridLineColor());
    }

    ctx.popClip();
    ctx.restore();

    // ---- Scrollbar ----
    if (scroll) {
        f32 sbX = bw - kScrollbarWidth;
        f32 sbY = hdrH;
        f32 sbH = bodyH;

        // Track
        ctx.fillRect({sbX, sbY, kScrollbarWidth, sbH}, scrollbarTrackColor());

        // Thumb
        f32 viewRatio = bodyH / totalRowsHeight();
        f32 thumbH = std::max(20.0f, sbH * viewRatio);
        f32 scrollMax = maxScrollOffset();
        f32 scrollRatio = (scrollMax > 0) ? m_scrollOffset / scrollMax : 0.0f;
        f32 thumbY = sbY + scrollRatio * (sbH - thumbH);
        ctx.fillRoundedRect({sbX, thumbY, kScrollbarWidth, thumbH},
                            kScrollbarWidth * 0.5f, scrollbarThumbColor());
    }
}

// ---- Mouse ------------------------------------------------------------------

Rectf Table::scrollbarThumbRect() const {
    f32 bw = bounds().width;
    f32 hdrH = headerHeight();
    f32 bH = bodyHeight();
    f32 sbX = bw - kScrollbarWidth;
    f32 viewRatio = bH / totalRowsHeight();
    f32 thumbH = std::max(20.0f, bH * viewRatio);
    f32 scrollMax = maxScrollOffset();
    f32 scrollRatio = (scrollMax > 0) ? m_scrollOffset / scrollMax : 0.0f;
    f32 thumbY = hdrH + scrollRatio * (bH - thumbH);
    return {sbX, thumbY, kScrollbarWidth, thumbH};
}

bool Table::onMouseEvent(const MouseEvent& event) {
    if (!isEnabled()) return false;

    switch (event.type) {
        case MouseEventType::ButtonDown: {
            if (event.button != MouseButton::Left) break;

            // Scrollbar thumb grab
            if (needsScrollbar()) {
                Rectf thumb = scrollbarThumbRect();
                if (event.position.x >= thumb.x && event.position.x < thumb.x + thumb.width &&
                    event.position.y >= thumb.y && event.position.y < thumb.y + thumb.height) {
                    m_draggingThumb = true;
                    m_dragStartY = event.position.y;
                    m_dragStartScroll = m_scrollOffset;
                    if (context()) context()->inputManager().captureMouse(this);
                    return true;
                }
                // Click in scrollbar track (but not on thumb) → jump
                f32 sbX = bounds().width - kScrollbarWidth;
                if (event.position.x >= sbX && event.position.y >= headerHeight()) {
                    f32 bH = bodyHeight();
                    f32 viewRatio = bH / totalRowsHeight();
                    f32 thumbH = std::max(20.0f, bH * viewRatio);
                    f32 clickRatio = (event.position.y - headerHeight() - thumbH * 0.5f) / (bH - thumbH);
                    clickRatio = std::clamp(clickRatio, 0.0f, 1.0f);
                    m_scrollOffset = clickRatio * maxScrollOffset();
                    invalidateRender();
                    return true;
                }
            }

            // Header click → sort
            if (event.position.y < headerHeight()) {
                isize col = columnIndexAtX(event.position.x);
                if (col >= 0) {
                    bool asc = (sortColumn() == col) ? !sortAscending() : true;
                    setsortColumn(col);
                    setsortAscending(asc);
                    if (m_onSortRequested)
                        m_onSortRequested(static_cast<usize>(col), asc);
                    invalidateRender();
                    return true;
                }
            }

            // Body click → select row
            isize row = rowIndexAtY(event.position.y);
            if (row >= 0) {
                selectRow(row);
                return true;
            }
            break;
        }

        case MouseEventType::ButtonUp: {
            if (event.button == MouseButton::Left && m_draggingThumb) {
                m_draggingThumb = false;
                if (context()) context()->inputManager().releaseMouse();
                return true;
            }
            break;
        }

        case MouseEventType::Move: {
            // Scrollbar thumb drag
            if (m_draggingThumb) {
                f32 bH = bodyHeight();
                f32 viewRatio = bH / totalRowsHeight();
                f32 thumbH = std::max(20.0f, bH * viewRatio);
                f32 trackRange = bH - thumbH;
                if (trackRange > 0) {
                    f32 dy = event.position.y - m_dragStartY;
                    f32 scrollDelta = (dy / trackRange) * maxScrollOffset();
                    m_scrollOffset = std::clamp(m_dragStartScroll + scrollDelta, 0.0f, maxScrollOffset());
                    invalidateRender();
                }
                return true;
            }

            // Header hover
            isize newHdrCol = -1;
            if (event.position.y < headerHeight()) {
                newHdrCol = columnIndexAtX(event.position.x);
            }
            if (newHdrCol != m_hoveredHeaderCol) {
                m_hoveredHeaderCol = newHdrCol;
                invalidateRender();
            }

            // Row hover
            isize newRow = rowIndexAtY(event.position.y);
            if (newRow != m_hoveredRow) {
                m_hoveredRow = newRow;
                invalidateRender();
            }
            break;
        }

        case MouseEventType::Wheel: {
            if (!needsScrollbar()) break;
            m_scrollOffset -= event.delta.y;
            m_scrollOffset = std::clamp(m_scrollOffset, 0.0f, maxScrollOffset());
            invalidateRender();
            return true;
        }

        default:
            break;
    }
    return false;
}

// ---- Keyboard ---------------------------------------------------------------

bool Table::onKeyEvent(const KeyEvent& event) {
    if (event.type != KeyEventType::KeyDown) return false;
    isize count = static_cast<isize>(m_rows.size());
    if (count == 0) return false;

    switch (event.key) {
        case Key::Up: {
            isize next = selectedRow() - 1;
            if (next < 0) next = 0;
            selectRow(next);
            // Scroll to keep visible
            f32 ry = static_cast<f32>(next) * rowHeight();
            if (ry < m_scrollOffset) m_scrollOffset = ry;
            invalidateRender();
            return true;
        }
        case Key::Down: {
            isize next = selectedRow() + 1;
            if (next >= count) next = count - 1;
            selectRow(next);
            f32 ryBottom = static_cast<f32>(next + 1) * rowHeight();
            if (ryBottom > m_scrollOffset + bodyHeight()) {
                m_scrollOffset = ryBottom - bodyHeight();
            }
            invalidateRender();
            return true;
        }
        case Key::Home: {
            selectRow(0);
            m_scrollOffset = 0;
            invalidateRender();
            return true;
        }
        case Key::End: {
            selectRow(count - 1);
            m_scrollOffset = maxScrollOffset();
            invalidateRender();
            return true;
        }
        case Key::PageUp: {
            f32 page = bodyHeight();
            isize rows = static_cast<isize>(page / rowHeight());
            isize next = std::max<isize>(0, selectedRow() - rows);
            selectRow(next);
            m_scrollOffset = std::max(0.0f, m_scrollOffset - page);
            invalidateRender();
            return true;
        }
        case Key::PageDown: {
            f32 page = bodyHeight();
            isize rows = static_cast<isize>(page / rowHeight());
            isize next = std::min<isize>(count - 1, selectedRow() + rows);
            selectRow(next);
            m_scrollOffset = std::min(maxScrollOffset(), m_scrollOffset + page);
            invalidateRender();
            return true;
        }
        default:
            break;
    }
    return false;
}

// ---- Mouse enter / leave ----------------------------------------------------

void Table::onMouseEnter() {
    Element::onMouseEnter();
}

void Table::onMouseLeave() {
    if (m_hoveredRow != -1 || m_hoveredHeaderCol != -1) {
        m_hoveredRow = -1;
        m_hoveredHeaderCol = -1;
        invalidateRender();
    }
    Element::onMouseLeave();
}

} // namespace gut


// === elements/ListView.cpp ===================================================

#include <algorithm>
#include <cmath>

namespace gut {

ListView::ListView() {
    setfocusable(true);
    setcursor(CursorType::Arrow);
}

// ---- Data -------------------------------------------------------------------

void ListView::setItemCount(isize count) {
    m_itemCount = std::max<isize>(0, count);
    m_selectedIndices.clear();
    m_anchorIndex = -1;
    m_hoveredItem = -1;
    m_scrollOffset = std::clamp(m_scrollOffset, 0.0f, maxScrollOffset());
    invalidateLayout();
    invalidateRender();
}

void ListView::refresh() {
    m_scrollOffset = std::clamp(m_scrollOffset, 0.0f, maxScrollOffset());
    invalidateRender();
}

// ---- Selection --------------------------------------------------------------

isize ListView::selectedIndex() const {
    return m_selectedIndices.empty() ? -1 : *m_selectedIndices.begin();
}

void ListView::selectIndex(isize index) {
    if (selectionMode() == SelectionMode::None) return;
    if (index < 0 || index >= m_itemCount) return;
    m_selectedIndices.clear();
    m_selectedIndices.insert(index);
    m_anchorIndex = index;
    ensureVisible(index);
    notifySelectionChanged();
    invalidateRender();
}

void ListView::deselectAll() {
    if (m_selectedIndices.empty()) return;
    m_selectedIndices.clear();
    notifySelectionChanged();
    invalidateRender();
}

void ListView::selectRange(isize from, isize to) {
    if (selectionMode() == SelectionMode::None) return;
    isize lo = std::max<isize>(0, std::min(from, to));
    isize hi = std::min(m_itemCount - 1, std::max(from, to));
    for (isize i = lo; i <= hi; ++i) m_selectedIndices.insert(i);
    notifySelectionChanged();
    invalidateRender();
}

void ListView::notifySelectionChanged() {
    if (m_onSelectionChanged) m_onSelectionChanged(m_selectedIndices);
}

void ListView::ensureVisible(isize index) {
    if (index < 0 || index >= m_itemCount) return;
    f32 itemTop = static_cast<f32>(index) * itemHeight();
    f32 itemBot = itemTop + itemHeight();
    if (itemTop < m_scrollOffset) {
        m_scrollOffset = itemTop;
    } else if (itemBot > m_scrollOffset + viewHeight()) {
        m_scrollOffset = itemBot - viewHeight();
    }
    m_scrollOffset = std::clamp(m_scrollOffset, 0.0f, maxScrollOffset());
}

// ---- Geometry ---------------------------------------------------------------

isize ListView::itemIndexAtY(f32 localY) const {
    if (localY < 0) return -1;
    f32 yInContent = localY + m_scrollOffset;
    isize idx = static_cast<isize>(yInContent / itemHeight());
    if (idx < 0 || idx >= m_itemCount) return -1;
    return idx;
}

Rectf ListView::scrollbarThumbRect() const {
    f32 bw = bounds().width;
    f32 bh = viewHeight();
    f32 sbX = bw - kScrollbarWidth;
    f32 viewR = bh / totalHeight();
    f32 thumbH = std::max(20.0f, bh * viewR);
    f32 scrollMax = maxScrollOffset();
    f32 scrollR = (scrollMax > 0) ? m_scrollOffset / scrollMax : 0.0f;
    f32 thumbY = scrollR * (bh - thumbH);
    return {sbX, thumbY, kScrollbarWidth, thumbH};
}

// ---- Layout -----------------------------------------------------------------

Size2f ListView::measureOverride(Size2f /*availableSize*/) {
    f32 w = (width() == width()) ? width() : 200.0f;
    f32 h = (height() == height()) ? height() : totalHeight();
    return {w, h};
}

// ---- Render -----------------------------------------------------------------

void ListView::onRender(RenderContext& ctx) {
    f32 bw = bounds().width;
    f32 bh = bounds().height;
    bool scroll = needsScrollbar();
    f32 contentW = scroll ? bw - kScrollbarWidth : bw;

    // Background
    ctx.fillRect({0, 0, bw, bh}, listBackground());

    // Determine visible range
    f32 ih = itemHeight();
    isize firstVisible = static_cast<isize>(m_scrollOffset / ih);
    isize lastVisible  = static_cast<isize>((m_scrollOffset + bh) / ih);
    lastVisible = std::min(lastVisible, m_itemCount - 1);

    Font* font = context() ? context()->defaultFont() : nullptr;
    Ref<FontFace> face;
    if (font) face = font->getFace(fontSize());

    ctx.save();
    ctx.pushClip({0, 0, contentW, bh});

    for (isize i = firstVisible; i <= lastVisible; ++i) {
        f32 ry = static_cast<f32>(i) * ih - m_scrollOffset;

        // Row background  —  alt / hover / selected
        bool isSelected = m_selectedIndices.count(i) > 0;
        Color bg = (i % 2 == 0) ? itemBackground() : itemAlternateBackground();
        if (i == m_hoveredItem && !isSelected) bg = itemHoverBackground();
        if (isSelected) bg = itemSelectedBackground();
        if (bg.a > 0.0f) {
            ctx.fillRect({0, ry, contentW, ih}, bg);
        }

        // Item content — use template if provided, else fall back to plain index text
        if (m_itemTemplate) {
            Ref<Element> elem = m_itemTemplate(i);
            if (elem) {
                // Measure and arrange inside the row rect
                elem->measure({contentW, ih});
                elem->arrange({0, ry, contentW, ih});
                // Provide context so it can render fonts etc.
                if (!elem->context() && context()) elem->setContext(context());
                elem->render(ctx);
            }
        } else if (face) {
            // Default: render "Item N"
            String label = "Item " + std::to_string(i);
            Color fg = itemForeground();
            f32 tx = 10.0f;
            f32 ty = ry + (ih - face->lineHeight()) * 0.5f + face->ascender();
            ctx.drawText(face.get(), label, {tx, ty}, fg);
        }

        // Divider
        if (dividerColor().a > 0) {
            ctx.fillRect({0, ry + ih - 1.0f, contentW, 1.0f}, dividerColor());
        }
    }

    ctx.popClip();
    ctx.restore();

    // ---- Scrollbar ----
    if (scroll) {
        Rectf thumb = scrollbarThumbRect();
        ctx.fillRect({bw - kScrollbarWidth, 0, kScrollbarWidth, bh}, scrollbarTrackColor());
        ctx.fillRoundedRect(thumb, kScrollbarWidth * 0.5f, scrollbarThumbColor());
    }
}

// ---- Mouse ------------------------------------------------------------------

bool ListView::onMouseEvent(const MouseEvent& event) {
    if (!isEnabled()) return false;

    switch (event.type) {
        case MouseEventType::ButtonDown: {
            if (event.button != MouseButton::Left) break;

            // Scrollbar thumb grab
            if (needsScrollbar()) {
                Rectf thumb = scrollbarThumbRect();
                if (event.position.x >= thumb.x && event.position.x < thumb.x + thumb.width &&
                    event.position.y >= thumb.y && event.position.y < thumb.y + thumb.height) {
                    m_draggingThumb = true;
                    m_dragStartY = event.position.y;
                    m_dragStartScroll = m_scrollOffset;
                    if (context()) context()->inputManager().captureMouse(this);
                    return true;
                }
                // Click in scrollbar track → jump
                f32 sbX = bounds().width - kScrollbarWidth;
                if (event.position.x >= sbX) {
                    f32 bH = viewHeight();
                    f32 viewR = bH / totalHeight();
                    f32 thumbH = std::max(20.0f, bH * viewR);
                    f32 clickR = (event.position.y - thumbH * 0.5f) / (bH - thumbH);
                    clickR = std::clamp(clickR, 0.0f, 1.0f);
                    m_scrollOffset = clickR * maxScrollOffset();
                    invalidateRender();
                    return true;
                }
            }

            // Item click — selection logic
            isize idx = itemIndexAtY(event.position.y);
            if (idx >= 0 && selectionMode() != SelectionMode::None) {
                bool ctrl  = event.hasControl();
                bool shift = event.hasShift();

                switch (selectionMode()) {
                    case SelectionMode::Single:
                        m_selectedIndices.clear();
                        m_selectedIndices.insert(idx);
                        m_anchorIndex = idx;
                        break;

                    case SelectionMode::Multiple:
                        // Toggle with or without Ctrl
                        if (m_selectedIndices.count(idx)) {
                            m_selectedIndices.erase(idx);
                        } else {
                            m_selectedIndices.insert(idx);
                        }
                        m_anchorIndex = idx;
                        break;

                    case SelectionMode::Extended:
                        if (shift && m_anchorIndex >= 0) {
                            // Range from anchor to idx
                            if (!ctrl) m_selectedIndices.clear();
                            isize lo = std::min(m_anchorIndex, idx);
                            isize hi = std::max(m_anchorIndex, idx);
                            for (isize j = lo; j <= hi; ++j) m_selectedIndices.insert(j);
                        } else if (ctrl) {
                            // Toggle single
                            if (m_selectedIndices.count(idx)) {
                                m_selectedIndices.erase(idx);
                            } else {
                                m_selectedIndices.insert(idx);
                            }
                            m_anchorIndex = idx;
                        } else {
                            // Plain click — single select
                            m_selectedIndices.clear();
                            m_selectedIndices.insert(idx);
                            m_anchorIndex = idx;
                        }
                        break;

                    default:
                        break;
                }

                ensureVisible(idx);
                notifySelectionChanged();
                invalidateRender();
                return true;
            }
            break;
        }

        case MouseEventType::ButtonUp: {
            if (event.button == MouseButton::Left && m_draggingThumb) {
                m_draggingThumb = false;
                if (context()) context()->inputManager().releaseMouse();
                return true;
            }
            break;
        }

        case MouseEventType::DoubleClick: {
            isize idx = itemIndexAtY(event.position.y);
            if (idx >= 0 && m_onItemDoubleClicked) {
                m_onItemDoubleClicked(idx);
                return true;
            }
            break;
        }

        case MouseEventType::Move: {
            if (m_draggingThumb) {
                f32 bH = viewHeight();
                f32 viewR = bH / totalHeight();
                f32 thumbH = std::max(20.0f, bH * viewR);
                f32 trackRange = bH - thumbH;
                if (trackRange > 0) {
                    f32 dy = event.position.y - m_dragStartY;
                    f32 scrollDelta = (dy / trackRange) * maxScrollOffset();
                    m_scrollOffset = std::clamp(m_dragStartScroll + scrollDelta, 0.0f, maxScrollOffset());
                    invalidateRender();
                }
                return true;
            }

            isize idx = itemIndexAtY(event.position.y);
            if (idx != m_hoveredItem) {
                m_hoveredItem = idx;
                invalidateRender();
            }
            break;
        }

        case MouseEventType::Wheel: {
            if (!needsScrollbar()) break;
            m_scrollOffset -= event.delta.y;
            m_scrollOffset = std::clamp(m_scrollOffset, 0.0f, maxScrollOffset());
            invalidateRender();
            return true;
        }

        default:
            break;
    }
    return false;
}

// ---- Keyboard ---------------------------------------------------------------

bool ListView::onKeyEvent(const KeyEvent& event) {
    if (event.type != KeyEventType::KeyDown) return false;
    if (m_itemCount == 0 || selectionMode() == SelectionMode::None) return false;

    // For keyboard nav we always work with a single "cursor" index
    isize cur = selectedIndex();

    switch (event.key) {
        case Key::Up: {
            isize next = std::max<isize>(0, cur - 1);
            if (event.hasShift() && selectionMode() == SelectionMode::Extended && m_anchorIndex >= 0) {
                m_selectedIndices.clear();
                isize lo = std::min(m_anchorIndex, next);
                isize hi = std::max(m_anchorIndex, next);
                for (isize j = lo; j <= hi; ++j) m_selectedIndices.insert(j);
            } else {
                m_selectedIndices.clear();
                m_selectedIndices.insert(next);
                m_anchorIndex = next;
            }
            ensureVisible(next);
            notifySelectionChanged();
            invalidateRender();
            return true;
        }
        case Key::Down: {
            isize next = std::min(m_itemCount - 1, cur + 1);
            if (event.hasShift() && selectionMode() == SelectionMode::Extended && m_anchorIndex >= 0) {
                m_selectedIndices.clear();
                isize lo = std::min(m_anchorIndex, next);
                isize hi = std::max(m_anchorIndex, next);
                for (isize j = lo; j <= hi; ++j) m_selectedIndices.insert(j);
            } else {
                m_selectedIndices.clear();
                m_selectedIndices.insert(next);
                m_anchorIndex = next;
            }
            ensureVisible(next);
            notifySelectionChanged();
            invalidateRender();
            return true;
        }
        case Key::Home: {
            m_selectedIndices.clear();
            m_selectedIndices.insert(static_cast<isize>(0));
            m_anchorIndex = 0;
            m_scrollOffset = 0;
            notifySelectionChanged();
            invalidateRender();
            return true;
        }
        case Key::End: {
            isize last = m_itemCount - 1;
            m_selectedIndices.clear();
            m_selectedIndices.insert(last);
            m_anchorIndex = last;
            m_scrollOffset = maxScrollOffset();
            notifySelectionChanged();
            invalidateRender();
            return true;
        }
        case Key::PageUp: {
            f32 page = viewHeight();
            isize rows = std::max<isize>(1, static_cast<isize>(page / itemHeight()));
            isize next = std::max<isize>(0, cur - rows);
            m_selectedIndices.clear();
            m_selectedIndices.insert(next);
            m_anchorIndex = next;
            ensureVisible(next);
            notifySelectionChanged();
            invalidateRender();
            return true;
        }
        case Key::PageDown: {
            f32 page = viewHeight();
            isize rows = std::max<isize>(1, static_cast<isize>(page / itemHeight()));
            isize next = std::min(m_itemCount - 1, cur + rows);
            m_selectedIndices.clear();
            m_selectedIndices.insert(next);
            m_anchorIndex = next;
            ensureVisible(next);
            notifySelectionChanged();
            invalidateRender();
            return true;
        }
        case Key::A: {
            // Ctrl+A → select all (in Multiple or Extended mode)
            if (event.hasControl() &&
                (selectionMode() == SelectionMode::Multiple || selectionMode() == SelectionMode::Extended)) {
                m_selectedIndices.clear();
                for (isize j = 0; j < m_itemCount; ++j) m_selectedIndices.insert(j);
                notifySelectionChanged();
                invalidateRender();
                return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}

// ---- Mouse enter / leave ----------------------------------------------------

void ListView::onMouseEnter() {
    Element::onMouseEnter();
}

void ListView::onMouseLeave() {
    if (m_hoveredItem != -1) {
        m_hoveredItem = -1;
        invalidateRender();
    }
    Element::onMouseLeave();
}

} // namespace gut


// --- elements/Dialog.cpp ---

#include <cmath>
#include <algorithm>

namespace gut {

Dialog::Dialog() {
    setfocusable(true);
    setcursor(CursorType::Arrow);
}

void Dialog::setContent(Ref<Element> content) {
    m_content = std::move(content);
    invalidateRender();
}

void Dialog::show() {
    if (m_visible) return;
    m_visible = true;
    m_dialogOffsetX = 0.0f;
    m_dialogOffsetY = 0.0f;
    m_closeHovered = false;
    m_hoveredButton = -1;
    m_dragging = false;

    if (context()) {
        context()->inputManager().captureMouse(this);
        context()->addOverlay(this, [this](RenderContext& ctx) {
            renderOverlay(ctx);
        });
        context()->focusManager().setFocus(this);
    }
    invalidateRender();
}

void Dialog::close(DialogResult result) {
    if (!m_visible) return;
    m_visible = false;
    m_dragging = false;

    if (context()) {
        context()->inputManager().releaseMouse();
        context()->removeOverlay(this);
    }
    if (m_onResult) m_onResult(result);
    if (m_onClosed) m_onClosed();
    invalidateRender();
}

Rectf Dialog::dialogRect() const {
    if (!context()) return {};
    Size2f win = context()->size();
    f32 dw = dialogWidth();
    f32 dh = dialogHeight();
    f32 x = (win.width - dw) * 0.5f + m_dialogOffsetX;
    f32 y = (win.height - dh) * 0.5f + m_dialogOffsetY;
    return {x, y, dw, dh};
}

Rectf Dialog::titleBarRect() const {
    return {0, 0, dialogWidth(), titleBarHeight()};
}

Rectf Dialog::closeButtonRect() const {
    f32 sz = kCloseButtonSize;
    f32 cx = dialogWidth() - kPadding;
    f32 cy = titleBarHeight() * 0.5f;
    return {cx - sz * 0.5f, cy - sz * 0.5f, sz, sz};
}

Rectf Dialog::bodyRect() const {
    f32 top = titleBarHeight();
    f32 bottom = (buttons() != DialogButtons::None) ? buttonAreaHeight() : 0.0f;
    return {kPadding, top + 10.0f, dialogWidth() - kPadding * 2, dialogHeight() - top - bottom - 10.0f};
}

Rectf Dialog::buttonAreaRect() const {
    f32 h = buttonAreaHeight();
    return {0, dialogHeight() - h, dialogWidth(), h};
}

std::vector<Dialog::ButtonInfo> Dialog::buttonLayout() const {
    std::vector<ButtonInfo> btns;
    f32 bw = buttonWidth();
    f32 bh = buttonHeight();
    f32 sp = buttonSpacing();

    switch (buttons()) {
        case DialogButtons::None:
            break;
        case DialogButtons::OK:
            btns.push_back({{0, 0, bw, bh}, "OK", true, DialogResult::OK});
            break;
        case DialogButtons::OKCancel:
            btns.push_back({{0, 0, bw, bh}, "OK", true, DialogResult::OK});
            btns.push_back({{0, 0, bw, bh}, "Cancel", false, DialogResult::Cancel});
            break;
        case DialogButtons::YesNo:
            btns.push_back({{0, 0, bw, bh}, "Yes", true, DialogResult::Yes});
            btns.push_back({{0, 0, bw, bh}, "No", false, DialogResult::No});
            break;
        case DialogButtons::YesNoCancel:
            btns.push_back({{0, 0, bw, bh}, "Yes", true, DialogResult::Yes});
            btns.push_back({{0, 0, bw, bh}, "No", false, DialogResult::No});
            btns.push_back({{0, 0, bw, bh}, "Cancel", false, DialogResult::Cancel});
            break;
    }

    // Center buttons in button area
    f32 totalW = btns.empty() ? 0.0f : (static_cast<f32>(btns.size()) * bw + static_cast<f32>(btns.size() - 1) * sp);
    Rectf ba = buttonAreaRect();
    f32 startX = ba.x + (ba.width - totalW) * 0.5f;
    f32 cy = ba.y + (ba.height - bh) * 0.5f;

    for (usize i = 0; i < btns.size(); ++i) {
        btns[i].rect.x = startX + static_cast<f32>(i) * (bw + sp);
        btns[i].rect.y = cy;
    }

    return btns;
}

Size2f Dialog::measureOverride(Size2f availableSize) {
    return {0, 0}; // Dialog doesn't occupy layout space; it's an overlay
}

void Dialog::onRender(RenderContext& /*ctx*/) {
    // Rendering happens via the overlay system — nothing here
}

void Dialog::renderOverlay(RenderContext& ctx) {
    if (!m_visible || !context()) return;

    Size2f win = context()->size();

    // --- Backdrop ---
    ctx.fillRect({0, 0, win.width, win.height}, backdropColor());

    // --- Dialog box ---
    Rectf dr = dialogRect();
    f32 cr = cornerRadius();

    // Shadow
    ctx.drawDropShadow(dr, cr, Color::fromRgba8(0, 0, 0, 140), 24.0f, 0.0f, 8.0f);

    // Background
    ctx.fillRoundedRect(dr, cr, dialogBackground());

    // Save and translate into dialog-local space
    ctx.save();
    ctx.translate(dr.x, dr.y);

    // --- Title bar ---
    {
        Rectf tb = titleBarRect();
        // Title bar background (top-rounded corners)
        ctx.save();
        ctx.pushClip({tb.x, tb.y, tb.width, tb.height}, cr);
        ctx.fillRect({tb.x, tb.y, tb.width, tb.height}, titleBarBackground());
        ctx.popClip();
        ctx.restore();

        // Separator line
        ctx.fillRect({0, tb.height - 1, tb.width, 1}, dialogBorderColor());

        // Title text
        Font* font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(titleFontSize());
            if (face) {
                f32 textY = (tb.height - face->lineHeight()) * 0.5f + face->ascender();
                ctx.drawText(face.get(), title(), {kPadding, textY}, titleForeground());
            }
        }

        // Close button (X)
        Rectf cb = closeButtonRect();
        Color xColor = m_closeHovered ? closeButtonHoverColor() : closeButtonColor();
        f32 inset = 3.0f;
        ctx.drawLine({cb.x + inset, cb.y + inset}, {cb.x + cb.width - inset, cb.y + cb.height - inset}, xColor, 2.0f);
        ctx.drawLine({cb.x + cb.width - inset, cb.y + inset}, {cb.x + inset, cb.y + cb.height - inset}, xColor, 2.0f);
    }

    // --- Body (message text) ---
    {
        Rectf br = bodyRect();
        Font* font = context()->defaultFont();
        if (font && !message().empty()) {
            auto face = font->getFace(messageFontSize());
            if (face) {
                // Simple word-wrapping text rendering
                f32 lineH = face->lineHeight();
                f32 maxW = br.width;
                f32 curX = 0;
                f32 curY = face->ascender();
                const String& msg = message();

                // Split into words and wrap
                usize i = 0;
                while (i < msg.size()) {
                    // Skip leading spaces
                    while (i < msg.size() && msg[i] == ' ') ++i;
                    if (i >= msg.size()) break;

                    // Check for newline
                    if (msg[i] == '\n') {
                        curX = 0;
                        curY += lineH;
                        ++i;
                        continue;
                    }

                    // Extract word
                    usize start = i;
                    while (i < msg.size() && msg[i] != ' ' && msg[i] != '\n') ++i;
                    String word = msg.substr(start, i - start);

                    f32 wordW = face->measureWidth(word);
                    f32 spaceW = face->measureWidth(" ");

                    if (curX > 0 && curX + spaceW + wordW > maxW) {
                        // Wrap to next line
                        curX = 0;
                        curY += lineH;
                    }

                    if (curX > 0) {
                        curX += spaceW;
                    }

                    if (br.y + curY < br.y + br.height) {
                        ctx.drawText(face.get(), word, {br.x + curX, br.y + curY}, messageForeground());
                    }
                    curX += wordW;
                }
            }
        }
    }

    // --- Buttons ---
    {
        auto btns = buttonLayout();
        Font* font = context()->defaultFont();
        Ref<FontFace> face;
        if (font) face = font->getFace(buttonFontSize());

        for (isize i = 0; i < static_cast<isize>(btns.size()); ++i) {
            const auto& btn = btns[static_cast<usize>(i)];
            bool hovered = (i == m_hoveredButton);
            Color bg, fg;

            if (btn.primary) {
                bg = hovered ? primaryButtonHoverBackground() : primaryButtonBackground();
                fg = primaryButtonForeground();
            } else {
                bg = hovered ? secondaryButtonHoverBackground() : secondaryButtonBackground();
                fg = secondaryButtonForeground();
            }

            f32 bcr = 5.0f;
            ctx.fillRoundedRect(btn.rect, bcr, bg);
            ctx.strokeRoundedRect(btn.rect, bcr, buttonBorderColor(), 1.0f);

            if (face) {
                f32 tw = face->measureWidth(btn.label);
                f32 tx = btn.rect.x + (btn.rect.width - tw) * 0.5f;
                f32 ty = btn.rect.y + (btn.rect.height - face->lineHeight()) * 0.5f + face->ascender();
                ctx.drawText(face.get(), btn.label, {tx, ty}, fg);
            }
        }
    }

    ctx.restore();

    // --- Dialog border ---
    ctx.strokeRoundedRect(dr, cr, dialogBorderColor(), 1.0f);
}

bool Dialog::onMouseEvent(const MouseEvent& event) {
    if (!m_visible || !context()) return false;

    // Convert mouse position from element-local space to screen space.
    // Since mouse capture routes events to us in element-local space,
    // we need screen coords to test against dialog geometry.
    Rectf sb = screenBounds();
    f32 screenX = sb.x + event.position.x;
    f32 screenY = sb.y + event.position.y;

    Rectf dr = dialogRect();
    // Position relative to dialog box
    f32 dlgX = screenX - dr.x;
    f32 dlgY = screenY - dr.y;
    bool insideDialog = (screenX >= dr.x && screenX < dr.x + dr.width &&
                         screenY >= dr.y && screenY < dr.y + dr.height);

    switch (event.type) {
        case MouseEventType::ButtonDown: {
            if (event.button != MouseButton::Left) break;

            if (!insideDialog) {
                if (dismissOnBackdropClick()) {
                    close(DialogResult::Cancel);
                }
                return true; // consume — modal
            }

            // Close button
            Rectf cb = closeButtonRect();
            if (dlgX >= cb.x && dlgX < cb.x + cb.width &&
                dlgY >= cb.y && dlgY < cb.y + cb.height) {
                close(DialogResult::Cancel);
                return true;
            }

            // Check buttons
            auto btns = buttonLayout();
            for (auto& btn : btns) {
                if (dlgX >= btn.rect.x && dlgX < btn.rect.x + btn.rect.width &&
                    dlgY >= btn.rect.y && dlgY < btn.rect.y + btn.rect.height) {
                    close(btn.result);
                    return true;
                }
            }

            // Title bar drag
            Rectf tb = titleBarRect();
            if (dlgY >= tb.y && dlgY < tb.y + tb.height) {
                m_dragging = true;
                m_dragStartX = screenX;
                m_dragStartY = screenY;
                m_dragStartOffsetX = m_dialogOffsetX;
                m_dragStartOffsetY = m_dialogOffsetY;
            }

            return true;
        }

        case MouseEventType::ButtonUp: {
            if (m_dragging) {
                m_dragging = false;
            }
            return true;
        }

        case MouseEventType::Move: {
            if (m_dragging) {
                m_dialogOffsetX = m_dragStartOffsetX + (screenX - m_dragStartX);
                m_dialogOffsetY = m_dragStartOffsetY + (screenY - m_dragStartY);
                invalidateRender();
                return true;
            }

            // Update hover states
            bool needsRedraw = false;

            // Close button hover
            Rectf cb = closeButtonRect();
            bool closeHov = insideDialog &&
                            dlgX >= cb.x && dlgX < cb.x + cb.width &&
                            dlgY >= cb.y && dlgY < cb.y + cb.height;
            if (closeHov != m_closeHovered) {
                m_closeHovered = closeHov;
                needsRedraw = true;
            }

            // Button hover
            isize newHovBtn = -1;
            if (insideDialog) {
                auto btns = buttonLayout();
                for (isize i = 0; i < static_cast<isize>(btns.size()); ++i) {
                    const auto& btn = btns[static_cast<usize>(i)];
                    if (dlgX >= btn.rect.x && dlgX < btn.rect.x + btn.rect.width &&
                        dlgY >= btn.rect.y && dlgY < btn.rect.y + btn.rect.height) {
                        newHovBtn = i;
                        break;
                    }
                }
            }
            if (newHovBtn != m_hoveredButton) {
                m_hoveredButton = newHovBtn;
                needsRedraw = true;
            }

            if (needsRedraw) invalidateRender();
            return true; // consume — modal
        }

        case MouseEventType::Wheel:
            return true; // consume — modal, don't scroll behind

        default:
            break;
    }
    return true;
}

bool Dialog::onKeyEvent(const KeyEvent& event) {
    if (!m_visible) return false;
    if (!event.isKeyDown()) return true;

    if (event.key == Key::Escape) {
        close(DialogResult::Cancel);
        return true;
    }

    if (event.key == Key::Return) {
        // Press the primary (first) button
        auto btns = buttonLayout();
        for (auto& btn : btns) {
            if (btn.primary) {
                close(btn.result);
                return true;
            }
        }
        // No primary button — just close
        close(DialogResult::OK);
        return true;
    }

    return true; // consume all keys while modal
}

} // namespace gut


// === Tooltip implementation ===================================================

#include <unordered_map>

namespace gut {

// Internal state for the tooltip system
namespace {

struct TooltipState {
    // Map from element raw pointer to tooltip text
    std::unordered_map<Element*, String> tooltipTexts;
    // Map from element raw pointer to custom delay (0 = use global default)
    std::unordered_map<Element*, f32> tooltipDelays;

    // Currently showing tooltip for this element (nullptr if none)
    Element* activeElement{nullptr};
    // Position where the tooltip should appear (screen coords)
    f32 tipX{0.0f};
    f32 tipY{0.0f};
    // The context that owns the overlay
    Context* overlayContext{nullptr};
    // A dummy element used as overlay owner
    Ref<Element> overlayOwner;
};

TooltipState& tooltipState() {
    static TooltipState s;
    return s;
}

} // anonymous namespace

Tooltip::Style& Tooltip::style() {
    static Style s;
    return s;
}

void Tooltip::set(Ref<Element> element, String text) {
    set(std::move(element), std::move(text), 0.0f);
}

void Tooltip::set(Ref<Element> element, String text, f32 /*delayMs*/) {
    if (!element) return;

    auto& state = tooltipState();
    Element* raw = element.get();

    if (text.empty()) {
        // Remove tooltip
        state.tooltipTexts.erase(raw);
        state.tooltipDelays.erase(raw);
        if (state.activeElement == raw) {
            // Hide if showing
            if (state.overlayContext && state.overlayOwner) {
                state.overlayContext->removeOverlay(state.overlayOwner.get());
            }
            state.activeElement = nullptr;
            state.overlayContext = nullptr;
        }
        return;
    }

    state.tooltipTexts[raw] = text;

    // Connect mouseEntered / mouseLeft signals (only connect once)
    // We use a weak pattern: check if text exists in map
    element->mouseEntered().connect([raw]() {
        auto& st = tooltipState();
        auto it = st.tooltipTexts.find(raw);
        if (it == st.tooltipTexts.end()) return;
        if (!raw->context()) return;

        // Get mouse position from the input manager
        Point2f mp = raw->context()->inputManager().mousePosition();
        const auto& sty = Tooltip::style();

        // Position tooltip below cursor
        st.tipX = mp.x;
        st.tipY = mp.y + sty.offsetY;
        st.activeElement = raw;
        st.overlayContext = raw->context();

        if (!st.overlayOwner) {
            st.overlayOwner = make<Panel>();
        }

        st.overlayContext->addOverlay(st.overlayOwner.get(), [&st, raw](RenderContext& ctx) {
            auto it2 = st.tooltipTexts.find(raw);
            if (it2 == st.tooltipTexts.end() || st.activeElement != raw) return;

            const auto& sty2 = Tooltip::style();
            const String& tipText = it2->second;

            // Measure text
            Font* font = nullptr;
            Ref<FontFace> face;
            // We need access to the context's default font
            if (st.overlayContext) {
                font = st.overlayContext->defaultFont();
                if (font) face = font->getFace(sty2.fontSize);
            }
            if (!face) return;

            f32 textW = face->measureWidth(tipText);
            f32 textH = face->lineHeight();
            f32 boxW = textW + sty2.paddingH * 2.0f;
            f32 boxH = textH + sty2.paddingV * 2.0f;

            // Clamp within frame
            Size2f frame = ctx.frameSize();
            f32 bx = st.tipX - boxW * 0.5f; // center horizontally on cursor
            f32 by = st.tipY;
            if (bx < 4.0f) bx = 4.0f;
            if (bx + boxW > frame.width - 4.0f) bx = frame.width - 4.0f - boxW;
            if (by + boxH > frame.height - 4.0f) {
                // Show above cursor instead
                by = st.tipY - sty2.offsetY - boxH - 4.0f;
            }

            // Background
            ctx.fillRoundedRect({bx, by, boxW, boxH}, sty2.cornerRadius, sty2.background);
            ctx.strokeRoundedRect({bx, by, boxW, boxH}, sty2.cornerRadius, sty2.borderColor, 1.0f);

            // Text
            f32 tx = bx + sty2.paddingH;
            f32 ty = by + sty2.paddingV + face->ascender();
            ctx.drawText(face.get(), tipText, {tx, ty}, sty2.foreground);
        });
    });

    element->mouseLeft().connect([raw]() {
        auto& st = tooltipState();
        if (st.activeElement == raw && st.overlayContext && st.overlayOwner) {
            st.overlayContext->removeOverlay(st.overlayOwner.get());
            st.activeElement = nullptr;
            st.overlayContext = nullptr;
        }
    });
}

} // namespace gut


// === ContextMenu implementation ==============================================

namespace gut {

ContextMenu::ContextMenu() = default;

void ContextMenu::addItem(String label, std::function<void()> action, bool enabled) {
    m_items.push_back({std::move(label), std::move(action), false, enabled});
}

void ContextMenu::addSeparator() {
    m_items.push_back({"", nullptr, true, false});
}

void ContextMenu::clearItems() {
    m_items.clear();
}

void ContextMenu::attachTo(Ref<Element> element) {
    if (!element) return;
    // The user should call showAt() from their own right-click handler.
    // This method is a convenience placeholder for future event-filter support.
    // For now, it's a no-op — users wire up right-click manually in their code.
}

void ContextMenu::showAt(f32 x, f32 y) {
    if (!context()) return;
    m_popupX = x;
    m_popupY = y;
    m_isOpen = true;
    m_hoveredItemIndex = -1;

    context()->inputManager().captureMouse(this);
    context()->addOverlay(this, [this](RenderContext& ctx) {
        renderPopupOverlay(ctx);
    });
    invalidateRender();
}

void ContextMenu::close() {
    if (!m_isOpen) return;
    m_isOpen = false;

    if (context()) {
        context()->inputManager().releaseMouse();
        context()->removeOverlay(this);
    }

    if (m_onClosed) m_onClosed();
    invalidateRender();
}

f32 ContextMenu::totalMenuHeight() const {
    f32 h = 0;
    for (const auto& item : m_items) {
        h += item.separator ? separatorHeight() + 6.0f : itemHeight();
    }
    return h;
}

isize ContextMenu::itemIndexAtY(f32 localY) const {
    f32 y = 0;
    for (isize i = 0; i < static_cast<isize>(m_items.size()); ++i) {
        const auto& item = m_items[static_cast<usize>(i)];
        f32 ih = item.separator ? separatorHeight() + 6.0f : itemHeight();
        if (localY >= y && localY < y + ih) {
            return item.separator ? -1 : i;
        }
        y += ih;
    }
    return -1;
}

Size2f ContextMenu::measureOverride(Size2f /*availableSize*/) {
    return {menuWidth(), totalMenuHeight()};
}

void ContextMenu::onRender(RenderContext& /*ctx*/) {
    // Rendering is done via the overlay
}

void ContextMenu::renderPopupOverlay(RenderContext& ctx) {
    f32 mw = menuWidth();
    f32 mh = totalMenuHeight();
    f32 pad = 4.0f; // vertical padding inside menu
    f32 totalH = mh + pad * 2.0f;

    // Clamp position within frame
    Size2f frame = ctx.frameSize();
    f32 mx = m_popupX;
    f32 my = m_popupY;
    if (mx + mw > frame.width - 4.0f) mx = frame.width - 4.0f - mw;
    if (my + totalH > frame.height - 4.0f) my = frame.height - 4.0f - totalH;
    if (mx < 4.0f) mx = 4.0f;
    if (my < 4.0f) my = 4.0f;

    // Store adjusted position for hit testing
    m_popupX = mx;
    m_popupY = my;

    // Shadow
    ctx.fillRoundedRect({mx + 3.0f, my + 3.0f, mw, totalH}, cornerRadius(), Color::fromRgba8(0, 0, 0, 80));

    // Background
    ctx.fillRoundedRect({mx, my, mw, totalH}, cornerRadius(), menuBackground());
    ctx.strokeRoundedRect({mx, my, mw, totalH}, cornerRadius(), menuBorderColor(), 1.0f);

    // Items
    Font* font = context() ? context()->defaultFont() : nullptr;
    Ref<FontFace> face;
    if (font) face = font->getFace(fontSize());

    f32 iy = my + pad;
    for (isize i = 0; i < static_cast<isize>(m_items.size()); ++i) {
        const auto& item = m_items[static_cast<usize>(i)];

        if (item.separator) {
            f32 sepY = iy + 3.0f;
            ctx.fillRect({mx + 8.0f, sepY, mw - 16.0f, separatorHeight()}, separatorColor());
            iy += separatorHeight() + 6.0f;
            continue;
        }

        f32 ih = itemHeight();

        // Hover highlight
        bool hovered = (i == m_hoveredItemIndex && item.enabled);
        if (hovered) {
            f32 hlPad = 4.0f;
            ctx.fillRoundedRect({mx + hlPad, iy, mw - hlPad * 2.0f, ih}, 4.0f, itemHoverBackground());
        }

        // Text
        if (face) {
            Color fg;
            if (!item.enabled) fg = disabledForeground();
            else if (hovered) fg = itemHoverForeground();
            else fg = itemForeground();

            f32 tx = mx + 12.0f;
            f32 ty = iy + (ih - face->lineHeight()) * 0.5f + face->ascender();
            ctx.drawText(face.get(), item.label, {tx, ty}, fg);
        }

        iy += ih;
    }
}

bool ContextMenu::onMouseEvent(const MouseEvent& event) {
    if (!m_isOpen) return false;

    // Convert to screen coordinates
    Rectf sb = screenBounds();
    f32 sx = sb.x + event.position.x;
    f32 sy = sb.y + event.position.y;

    f32 pad = 4.0f;
    f32 mw = menuWidth();
    f32 mh = totalMenuHeight() + pad * 2.0f;

    bool inside = (sx >= m_popupX && sx < m_popupX + mw &&
                   sy >= m_popupY && sy < m_popupY + mh);

    switch (event.type) {
        case MouseEventType::ButtonDown: {
            if (!inside) {
                close();
                return true;
            }
            if (event.button == MouseButton::Left) {
                f32 localY = sy - m_popupY - pad;
                isize idx = itemIndexAtY(localY);
                if (idx >= 0) {
                    const auto& item = m_items[static_cast<usize>(idx)];
                    if (item.enabled && item.action) {
                        close();
                        item.action();
                        return true;
                    }
                }
            }
            return true;
        }
        case MouseEventType::Move: {
            if (inside) {
                f32 localY = sy - m_popupY - pad;
                isize idx = itemIndexAtY(localY);
                if (idx != m_hoveredItemIndex) {
                    m_hoveredItemIndex = idx;
                    invalidateRender();
                }
            } else {
                if (m_hoveredItemIndex != -1) {
                    m_hoveredItemIndex = -1;
                    invalidateRender();
                }
            }
            return true;
        }
        default:
            return true; // consume all mouse events while open
    }
}

bool ContextMenu::onKeyEvent(const KeyEvent& event) {
    if (!m_isOpen) return false;
    if (event.type != KeyEventType::KeyDown) return true;

    if (event.key == Key::Escape) {
        close();
        return true;
    }

    return true; // consume all keys while open
}

} // namespace gut


// === Toast implementation ====================================================

#include <vector>
#include <algorithm>

namespace gut {

namespace {

struct ToastEntry {
    String message;
    ToastPosition position;
    f64 showTime;     // context totalTime when shown
    f32 duration;     // seconds to stay visible
    Context* ctx;
};

// All active toasts, shared across contexts
struct ToastManager {
    std::vector<ToastEntry> entries;
    // Dummy overlay owner per context (one per context)
    std::unordered_map<Context*, Ref<Element>> overlayOwners;

    void ensureOverlay(Context* ctx) {
        if (overlayOwners.count(ctx)) return;
        auto owner = make<Panel>();
        ctx->addOverlay(owner.get(), [this, ctx](RenderContext& rc) {
            renderToasts(rc, ctx);
        });
        overlayOwners[ctx] = std::move(owner);
    }

    void removeOverlayIfEmpty(Context* ctx) {
        // Still have toasts for this context?
        for (auto& e : entries) {
            if (e.ctx == ctx) return;
        }
        auto it = overlayOwners.find(ctx);
        if (it != overlayOwners.end()) {
            ctx->removeOverlay(it->second.get());
            overlayOwners.erase(it);
        }
    }

    void renderToasts(RenderContext& rc, Context* ctx) {
        const auto& sty = Toast::style();
        f64 now = ctx->totalTime();

        // Expire old toasts
        entries.erase(
            std::remove_if(entries.begin(), entries.end(),
                [now](const ToastEntry& e) { return now - e.showTime >= e.duration; }),
            entries.end());

        if (entries.empty()) {
            // Schedule overlay removal (safe — we're inside render, so defer)
            return;
        }

        Size2f frame = rc.frameSize();
        Font* font = ctx->defaultFont();
        if (!font) return;
        auto face = font->getFace(sty.fontSize);
        if (!face) return;

        // Group toasts by position and render each stack
        for (u8 p = 0; p <= static_cast<u8>(ToastPosition::BottomRight); ++p) {
            auto pos = static_cast<ToastPosition>(p);

            // Collect toasts for this position + context
            std::vector<const ToastEntry*> stack;
            for (auto& e : entries) {
                if (e.ctx == ctx && e.position == pos) stack.push_back(&e);
            }
            if (stack.empty()) continue;

            // Determine anchor point
            bool top = (pos == ToastPosition::TopLeft || pos == ToastPosition::TopCenter || pos == ToastPosition::TopRight);
            bool left = (pos == ToastPosition::TopLeft || pos == ToastPosition::BottomLeft);
            bool center = (pos == ToastPosition::TopCenter || pos == ToastPosition::BottomCenter);

            f32 cursorY = top ? sty.margin : frame.height - sty.margin;

            for (auto* entry : stack) {
                f64 age = now - entry->showTime;
                // Fade in (first 150ms) and fade out (last 400ms)
                // age and entry->duration are both in milliseconds
                f32 alpha = 1.0f;
                constexpr f64 fadeInMs  = 150.0;
                constexpr f64 fadeOutMs = 400.0;
                if (age < fadeInMs) alpha = static_cast<f32>(age / fadeInMs);
                else if (age > entry->duration - fadeOutMs)
                    alpha = static_cast<f32>((entry->duration - age) / fadeOutMs);
                alpha = std::clamp(alpha, 0.0f, 1.0f);

                // Measure text (simple single-line for now, clamp to maxWidth)
                f32 textW = face->measureWidth(entry->message);
                if (textW > sty.maxWidth) textW = sty.maxWidth;
                f32 boxW = textW + sty.paddingH * 2.0f;
                f32 boxH = face->lineHeight() + sty.paddingV * 2.0f;

                // X position
                f32 bx;
                if (center) bx = (frame.width - boxW) * 0.5f;
                else if (left) bx = sty.margin;
                else bx = frame.width - sty.margin - boxW;

                // Y position (stack grows away from edge)
                f32 by;
                if (top) {
                    by = cursorY;
                    cursorY += boxH + sty.spacing;
                } else {
                    by = cursorY - boxH;
                    cursorY -= boxH + sty.spacing;
                }

                // Draw with alpha
                Color bg = sty.background;
                bg.a *= alpha;
                Color fg = sty.foreground;
                fg.a *= alpha;
                Color bc = sty.borderColor;
                bc.a *= alpha;

                // Shadow
                Color shadowC = Color::fromRgba8(0, 0, 0, static_cast<u8>(60.0f * alpha));
                rc.fillRoundedRect({bx + 2.0f, by + 2.0f, boxW, boxH}, sty.cornerRadius, shadowC);

                // Background + border
                rc.fillRoundedRect({bx, by, boxW, boxH}, sty.cornerRadius, bg);
                rc.strokeRoundedRect({bx, by, boxW, boxH}, sty.cornerRadius, bc, 1.0f);

                // Text (clip to maxWidth)
                f32 tx = bx + sty.paddingH;
                f32 ty = by + sty.paddingV + face->ascender();
                rc.save();
                rc.pushClip({bx + sty.paddingH, by + sty.paddingV,
                             boxW - sty.paddingH * 2.0f, boxH - sty.paddingV * 2.0f});
                rc.drawText(face.get(), entry->message, {tx, ty}, fg);
                rc.popClip();
                rc.restore();
            }
        }

        // Clean up overlay if all toasts expired
        removeOverlayIfEmpty(ctx);
    }
};

ToastManager& toastManager() {
    static ToastManager mgr;
    return mgr;
}

} // anonymous namespace

Toast::Style& Toast::style() {
    static Style s;
    return s;
}

void Toast::show(Context* ctx, String message, ToastPosition position, f32 durationSec) {
    if (!ctx) return;

    auto& mgr = toastManager();
    const auto& sty = Toast::style();

    f32 dur = durationSec > 0.0f ? durationSec : sty.defaultDuration;

    // totalTime() is in milliseconds (Context::update receives ms deltas),
    // so convert duration from seconds to milliseconds.
    f32 durMs = dur * 1000.0f;

    mgr.entries.push_back({std::move(message), position, ctx->totalTime(), durMs, ctx});
    mgr.ensureOverlay(ctx);
}

void Toast::dismissAll(Context* ctx) {
    if (!ctx) return;
    auto& mgr = toastManager();
    mgr.entries.erase(
        std::remove_if(mgr.entries.begin(), mgr.entries.end(),
            [ctx](const ToastEntry& e) { return e.ctx == ctx; }),
        mgr.entries.end());
    mgr.removeOverlayIfEmpty(ctx);
}

} // namespace gut


// === Icon implementation =====================================================

#include <cmath>

namespace gut {

void Icon::draw(RenderContext& ctx, IconName name, Point2f pos, f32 size, Color color, f32 strokeWeight) {
    draw(ctx, name, {pos.x, pos.y, size, size}, color, strokeWeight);
}

void Icon::draw(RenderContext& ctx, IconName name, Rectf b, Color color, f32 strokeWeight) {
    if (name == IconName::None) return;

    // Normalise into a unit square then scale
    f32 s = std::min(b.width, b.height); // icon fits in smaller dimension
    // Centre within bounds
    f32 ox = b.x + (b.width - s) * 0.5f;
    f32 oy = b.y + (b.height - s) * 0.5f;
    f32 t = strokeWeight > 0.0f ? strokeWeight : std::max(1.0f, s * 0.08f); // auto thickness

    // Helper lambdas for local coords [0..1] → pixel
    auto px = [&](f32 u) -> f32 { return ox + u * s; };
    auto py = [&](f32 v) -> f32 { return oy + v * s; };
    auto pt = [&](f32 u, f32 v) -> Point2f { return {px(u), py(v)}; };

    switch (name) {

    // ── File / Document ─────────────────────────────────────────────────
    case IconName::FileNew: {
        // Page with folded corner
        f32 fold = 0.3f;
        ctx.drawLine(pt(0.2f,0.05f), pt(0.2f,0.95f), color, t);
        ctx.drawLine(pt(0.2f,0.95f), pt(0.8f,0.95f), color, t);
        ctx.drawLine(pt(0.8f,0.95f), pt(0.8f, 0.05f+fold), color, t);
        ctx.drawLine(pt(0.8f, 0.05f+fold), pt(0.8f-fold, 0.05f), color, t);
        ctx.drawLine(pt(0.8f-fold, 0.05f), pt(0.2f, 0.05f), color, t);
        // Fold line
        ctx.drawLine(pt(0.8f-fold, 0.05f), pt(0.8f-fold, 0.05f+fold), color, t*0.7f);
        ctx.drawLine(pt(0.8f-fold, 0.05f+fold), pt(0.8f, 0.05f+fold), color, t*0.7f);
        break;
    }
    case IconName::FileOpen: {
        // Folder shape
        ctx.drawLine(pt(0.1f,0.25f), pt(0.1f,0.85f), color, t);
        ctx.drawLine(pt(0.1f,0.85f), pt(0.9f,0.85f), color, t);
        ctx.drawLine(pt(0.9f,0.85f), pt(0.9f,0.35f), color, t);
        ctx.drawLine(pt(0.9f,0.35f), pt(0.5f,0.35f), color, t);
        ctx.drawLine(pt(0.5f,0.35f), pt(0.4f,0.25f), color, t);
        ctx.drawLine(pt(0.4f,0.25f), pt(0.1f,0.25f), color, t);
        break;
    }
    case IconName::Save: {
        // Floppy disk
        f32 r = 0.06f;
        ctx.strokeRoundedRect({px(0.1f), py(0.1f), s*0.8f, s*0.8f}, s*r, color, t);
        // Metal slider
        ctx.fillRect({px(0.35f), py(0.1f), s*0.3f, s*0.25f}, color);
        // Label area
        ctx.fillRect({px(0.2f), py(0.55f), s*0.6f, s*0.02f}, color);
        ctx.fillRect({px(0.2f), py(0.62f), s*0.6f, s*0.02f}, color);
        ctx.fillRect({px(0.2f), py(0.69f), s*0.4f, s*0.02f}, color);
        break;
    }
    case IconName::SaveAs: {
        // Smaller floppy + pencil
        ctx.strokeRoundedRect({px(0.05f), py(0.15f), s*0.65f, s*0.7f}, s*0.04f, color, t);
        ctx.fillRect({px(0.25f), py(0.15f), s*0.25f, s*0.2f}, color);
        // Pencil overlay
        ctx.drawLine(pt(0.6f,0.7f), pt(0.9f,0.1f), color, t);
        ctx.drawLine(pt(0.55f,0.65f), pt(0.85f,0.05f), color, t);
        ctx.drawLine(pt(0.55f,0.65f), pt(0.6f,0.7f), color, t*0.7f);
        break;
    }

    // ── Edit ────────────────────────────────────────────────────────────
    case IconName::Cut: {
        // Scissors: two small circles + crossing lines
        f32 cr = 0.12f;
        ctx.strokeEllipse(pt(0.3f,0.8f), s*cr, s*cr, color, t);
        ctx.strokeEllipse(pt(0.7f,0.8f), s*cr, s*cr, color, t);
        ctx.drawLine(pt(0.3f,0.68f), pt(0.6f,0.2f), color, t);
        ctx.drawLine(pt(0.7f,0.68f), pt(0.4f,0.2f), color, t);
        break;
    }
    case IconName::Copy: {
        // Two overlapping pages
        ctx.strokeRoundedRect({px(0.25f), py(0.2f), s*0.55f, s*0.65f}, s*0.04f, color, t);
        ctx.drawLine(pt(0.2f,0.3f), pt(0.2f,0.85f), color, t);
        ctx.drawLine(pt(0.2f,0.85f), pt(0.7f,0.85f), color, t);
        ctx.drawLine(pt(0.2f,0.3f), pt(0.25f,0.3f), color, t);
        ctx.drawLine(pt(0.7f,0.85f), pt(0.7f,0.82f), color, t);
        break;
    }
    case IconName::Paste: {
        // Clipboard
        ctx.strokeRoundedRect({px(0.15f), py(0.2f), s*0.7f, s*0.7f}, s*0.04f, color, t);
        // Clip at top
        ctx.fillRoundedRect({px(0.35f), py(0.12f), s*0.3f, s*0.16f}, s*0.04f, color);
        // Lines on clipboard
        ctx.fillRect({px(0.25f), py(0.5f), s*0.5f, s*0.02f}, color);
        ctx.fillRect({px(0.25f), py(0.6f), s*0.5f, s*0.02f}, color);
        ctx.fillRect({px(0.25f), py(0.7f), s*0.35f, s*0.02f}, color);
        break;
    }
    case IconName::Undo: {
        // Curved arrow pointing left
        f32 cx = 0.55f, cy = 0.5f, r = 0.3f;
        std::vector<Point2f> arc;
        for (int i = 0; i <= 12; ++i) {
            f32 a = 3.14159f * 0.2f + (3.14159f * 1.3f) * static_cast<f32>(i) / 12.0f;
            arc.push_back(pt(cx + r * std::cos(a), cy + r * std::sin(a)));
        }
        ctx.drawPolyline(arc, color, t, false, LineJoin::Round, LineCap::Round);
        // Arrow head
        auto tip = arc.front();
        ctx.drawLine(tip, {tip.x + s*0.12f, tip.y - s*0.08f}, color, t);
        ctx.drawLine(tip, {tip.x + s*0.02f, tip.y + s*0.12f}, color, t);
        break;
    }
    case IconName::Redo: {
        // Curved arrow pointing right
        f32 cx = 0.45f, cy = 0.5f, r = 0.3f;
        std::vector<Point2f> arc;
        for (int i = 0; i <= 12; ++i) {
            f32 a = 3.14159f * 1.8f - (3.14159f * 1.3f) * static_cast<f32>(i) / 12.0f;
            arc.push_back(pt(cx + r * std::cos(a), cy + r * std::sin(a)));
        }
        ctx.drawPolyline(arc, color, t, false, LineJoin::Round, LineCap::Round);
        auto tip = arc.front();
        ctx.drawLine(tip, {tip.x - s*0.12f, tip.y - s*0.08f}, color, t);
        ctx.drawLine(tip, {tip.x - s*0.02f, tip.y + s*0.12f}, color, t);
        break;
    }
    case IconName::Delete: {
        // Trash can
        ctx.drawLine(pt(0.3f,0.25f), pt(0.7f,0.25f), color, t); // lid
        ctx.drawLine(pt(0.25f,0.25f), pt(0.75f,0.25f), color, t*1.2f); // rim
        ctx.drawLine(pt(0.32f,0.25f), pt(0.35f,0.85f), color, t);
        ctx.drawLine(pt(0.68f,0.25f), pt(0.65f,0.85f), color, t);
        ctx.drawLine(pt(0.35f,0.85f), pt(0.65f,0.85f), color, t);
        // Handle
        ctx.drawLine(pt(0.42f,0.25f), pt(0.42f,0.15f), color, t*0.8f);
        ctx.drawLine(pt(0.42f,0.15f), pt(0.58f,0.15f), color, t*0.8f);
        ctx.drawLine(pt(0.58f,0.15f), pt(0.58f,0.25f), color, t*0.8f);
        // Interior lines
        ctx.drawLine(pt(0.5f,0.35f), pt(0.5f,0.75f), color, t*0.7f);
        break;
    }

    // ── Text formatting ────────────────────────────────────────────────
    case IconName::Bold: {
        // Bold "B"
        f32 tw = t * 1.5f;
        ctx.drawLine(pt(0.3f,0.15f), pt(0.3f,0.85f), color, tw);
        ctx.drawLine(pt(0.3f,0.15f), pt(0.6f,0.15f), color, tw);
        ctx.drawLine(pt(0.3f,0.5f),  pt(0.65f,0.5f), color, tw);
        ctx.drawLine(pt(0.3f,0.85f), pt(0.65f,0.85f), color, tw);
        // Bumps
        std::vector<Point2f> top;
        for (int i = 0; i <= 6; ++i) {
            f32 a = -1.5708f + 3.14159f * static_cast<f32>(i) / 6.0f;
            top.push_back(pt(0.6f + 0.12f*std::cos(a), 0.325f + 0.175f*std::sin(a)));
        }
        ctx.drawPolyline(top, color, tw, false, LineJoin::Round, LineCap::Round);
        std::vector<Point2f> bot;
        for (int i = 0; i <= 6; ++i) {
            f32 a = -1.5708f + 3.14159f * static_cast<f32>(i) / 6.0f;
            bot.push_back(pt(0.65f + 0.13f*std::cos(a), 0.675f + 0.175f*std::sin(a)));
        }
        ctx.drawPolyline(bot, color, tw, false, LineJoin::Round, LineCap::Round);
        break;
    }
    case IconName::Italic: {
        // Slanted "I"
        ctx.drawLine(pt(0.35f,0.15f), pt(0.65f,0.15f), color, t);
        ctx.drawLine(pt(0.3f,0.85f),  pt(0.6f,0.85f), color, t);
        ctx.drawLine(pt(0.55f,0.15f), pt(0.4f,0.85f), color, t*1.2f);
        break;
    }
    case IconName::Underline: {
        // "U" shape + underline
        std::vector<Point2f> uShape;
        for (int i = 0; i <= 10; ++i) {
            f32 a = 3.14159f * static_cast<f32>(i) / 10.0f;
            uShape.push_back(pt(0.5f + 0.22f*std::cos(a + 3.14159f), 0.6f + 0.2f*std::sin(a + 3.14159f)));
        }
        ctx.drawPolyline(uShape, color, t*1.2f, false, LineJoin::Round, LineCap::Round);
        ctx.drawLine(pt(0.28f,0.2f), pt(0.28f,0.6f), color, t*1.2f);
        ctx.drawLine(pt(0.72f,0.2f), pt(0.72f,0.6f), color, t*1.2f);
        ctx.drawLine(pt(0.2f,0.9f), pt(0.8f,0.9f), color, t);
        break;
    }
    case IconName::Strikethrough: {
        // "S" with horizontal strike
        ctx.drawLine(pt(0.15f,0.5f), pt(0.85f,0.5f), color, t);
        std::vector<Point2f> sCurve;
        for (int i = 0; i <= 8; ++i) {
            f32 v = static_cast<f32>(i) / 8.0f;
            f32 u = 0.5f + 0.2f * std::sin(v * 3.14159f * 2.0f - 1.5708f);
            sCurve.push_back(pt(u, 0.15f + v * 0.7f));
        }
        ctx.drawPolyline(sCurve, color, t*1.1f, false, LineJoin::Round, LineCap::Round);
        break;
    }
    case IconName::AlignLeft: {
        ctx.fillRect({px(0.15f), py(0.2f), s*0.7f, s*0.04f}, color);
        ctx.fillRect({px(0.15f), py(0.38f), s*0.5f, s*0.04f}, color);
        ctx.fillRect({px(0.15f), py(0.56f), s*0.65f, s*0.04f}, color);
        ctx.fillRect({px(0.15f), py(0.74f), s*0.45f, s*0.04f}, color);
        break;
    }
    case IconName::AlignCenter: {
        ctx.fillRect({px(0.15f), py(0.2f), s*0.7f, s*0.04f}, color);
        ctx.fillRect({px(0.25f), py(0.38f), s*0.5f, s*0.04f}, color);
        ctx.fillRect({px(0.18f), py(0.56f), s*0.65f, s*0.04f}, color);
        ctx.fillRect({px(0.28f), py(0.74f), s*0.45f, s*0.04f}, color);
        break;
    }
    case IconName::AlignRight: {
        ctx.fillRect({px(0.15f), py(0.2f), s*0.7f, s*0.04f}, color);
        ctx.fillRect({px(0.35f), py(0.38f), s*0.5f, s*0.04f}, color);
        ctx.fillRect({px(0.2f), py(0.56f), s*0.65f, s*0.04f}, color);
        ctx.fillRect({px(0.4f), py(0.74f), s*0.45f, s*0.04f}, color);
        break;
    }

    // ── Navigation ──────────────────────────────────────────────────────
    case IconName::ArrowUp: {
        ctx.drawLine(pt(0.5f,0.2f), pt(0.5f,0.8f), color, t);
        ctx.drawLine(pt(0.5f,0.2f), pt(0.25f,0.45f), color, t);
        ctx.drawLine(pt(0.5f,0.2f), pt(0.75f,0.45f), color, t);
        break;
    }
    case IconName::ArrowDown: {
        ctx.drawLine(pt(0.5f,0.8f), pt(0.5f,0.2f), color, t);
        ctx.drawLine(pt(0.5f,0.8f), pt(0.25f,0.55f), color, t);
        ctx.drawLine(pt(0.5f,0.8f), pt(0.75f,0.55f), color, t);
        break;
    }
    case IconName::ArrowLeft: {
        ctx.drawLine(pt(0.2f,0.5f), pt(0.8f,0.5f), color, t);
        ctx.drawLine(pt(0.2f,0.5f), pt(0.45f,0.25f), color, t);
        ctx.drawLine(pt(0.2f,0.5f), pt(0.45f,0.75f), color, t);
        break;
    }
    case IconName::ArrowRight: {
        ctx.drawLine(pt(0.8f,0.5f), pt(0.2f,0.5f), color, t);
        ctx.drawLine(pt(0.8f,0.5f), pt(0.55f,0.25f), color, t);
        ctx.drawLine(pt(0.8f,0.5f), pt(0.55f,0.75f), color, t);
        break;
    }
    case IconName::Home: {
        // House: roof + body
        ctx.drawLine(pt(0.5f,0.1f), pt(0.1f,0.5f), color, t);
        ctx.drawLine(pt(0.5f,0.1f), pt(0.9f,0.5f), color, t);
        ctx.drawLine(pt(0.2f,0.45f), pt(0.2f,0.9f), color, t);
        ctx.drawLine(pt(0.8f,0.45f), pt(0.8f,0.9f), color, t);
        ctx.drawLine(pt(0.2f,0.9f), pt(0.8f,0.9f), color, t);
        // Door
        ctx.drawLine(pt(0.42f,0.9f), pt(0.42f,0.6f), color, t*0.8f);
        ctx.drawLine(pt(0.58f,0.9f), pt(0.58f,0.6f), color, t*0.8f);
        ctx.drawLine(pt(0.42f,0.6f), pt(0.58f,0.6f), color, t*0.8f);
        break;
    }
    case IconName::Search: {
        // Magnifying glass
        f32 cr = 0.25f;
        ctx.strokeEllipse(pt(0.42f,0.42f), s*cr, s*cr, color, t);
        ctx.drawLine(pt(0.6f,0.6f), pt(0.85f,0.85f), color, t*1.4f);
        break;
    }
    case IconName::Refresh: {
        // Circular arrow
        std::vector<Point2f> arc;
        for (int i = 0; i <= 16; ++i) {
            f32 a = -0.4f + 5.2f * static_cast<f32>(i) / 16.0f;
            arc.push_back(pt(0.5f + 0.3f*std::cos(a), 0.5f + 0.3f*std::sin(a)));
        }
        ctx.drawPolyline(arc, color, t, false, LineJoin::Round, LineCap::Round);
        auto tip = arc.back();
        ctx.drawLine(tip, {tip.x + s*0.1f, tip.y - s*0.07f}, color, t);
        ctx.drawLine(tip, {tip.x - s*0.02f, tip.y - s*0.12f}, color, t);
        break;
    }

    // ── Actions ─────────────────────────────────────────────────────────
    case IconName::Plus: {
        ctx.drawLine(pt(0.5f,0.15f), pt(0.5f,0.85f), color, t*1.2f);
        ctx.drawLine(pt(0.15f,0.5f), pt(0.85f,0.5f), color, t*1.2f);
        break;
    }
    case IconName::Minus: {
        ctx.drawLine(pt(0.15f,0.5f), pt(0.85f,0.5f), color, t*1.2f);
        break;
    }
    case IconName::Close: {
        ctx.drawLine(pt(0.2f,0.2f), pt(0.8f,0.8f), color, t*1.2f);
        ctx.drawLine(pt(0.8f,0.2f), pt(0.2f,0.8f), color, t*1.2f);
        break;
    }
    case IconName::Check: {
        ctx.drawLine(pt(0.15f,0.5f), pt(0.4f,0.78f), color, t*1.3f);
        ctx.drawLine(pt(0.4f,0.78f), pt(0.85f,0.22f), color, t*1.3f);
        break;
    }
    case IconName::Settings: {
        // Gear: circle + 6 notches
        f32 cr = 0.18f, nr = 0.32f;
        ctx.strokeEllipse(pt(0.5f,0.5f), s*cr, s*cr, color, t);
        for (int i = 0; i < 6; ++i) {
            f32 a = static_cast<f32>(i) * 3.14159f / 3.0f;
            f32 c1 = std::cos(a), s1 = std::sin(a);
            ctx.drawLine(pt(0.5f + cr*c1, 0.5f + cr*s1),
                         pt(0.5f + nr*c1, 0.5f + nr*s1), color, t*1.5f);
        }
        break;
    }
    case IconName::Menu: {
        // Hamburger — 3 lines
        ctx.fillRect({px(0.15f), py(0.22f), s*0.7f, s*0.06f}, color);
        ctx.fillRect({px(0.15f), py(0.47f), s*0.7f, s*0.06f}, color);
        ctx.fillRect({px(0.15f), py(0.72f), s*0.7f, s*0.06f}, color);
        break;
    }
    case IconName::MoreHorizontal: {
        f32 dr = s * 0.05f;
        ctx.fillEllipse(pt(0.25f,0.5f), dr, dr, color);
        ctx.fillEllipse(pt(0.5f,0.5f), dr, dr, color);
        ctx.fillEllipse(pt(0.75f,0.5f), dr, dr, color);
        break;
    }
    case IconName::MoreVertical: {
        f32 dr = s * 0.05f;
        ctx.fillEllipse(pt(0.5f,0.25f), dr, dr, color);
        ctx.fillEllipse(pt(0.5f,0.5f), dr, dr, color);
        ctx.fillEllipse(pt(0.5f,0.75f), dr, dr, color);
        break;
    }

    // ── Media ───────────────────────────────────────────────────────────
    case IconName::Play: {
        ctx.fillTriangle(pt(0.25f,0.15f), pt(0.25f,0.85f), pt(0.82f,0.5f), color);
        break;
    }
    case IconName::Pause: {
        ctx.fillRect({px(0.2f), py(0.15f), s*0.2f, s*0.7f}, color);
        ctx.fillRect({px(0.6f), py(0.15f), s*0.2f, s*0.7f}, color);
        break;
    }
    case IconName::Stop: {
        ctx.fillRect({px(0.2f), py(0.2f), s*0.6f, s*0.6f}, color);
        break;
    }
    case IconName::SkipForward: {
        ctx.fillTriangle(pt(0.15f,0.15f), pt(0.15f,0.85f), pt(0.6f,0.5f), color);
        ctx.fillRect({px(0.65f), py(0.15f), s*0.08f, s*0.7f}, color);
        break;
    }
    case IconName::SkipBack: {
        ctx.fillTriangle(pt(0.85f,0.15f), pt(0.85f,0.85f), pt(0.4f,0.5f), color);
        ctx.fillRect({px(0.27f), py(0.15f), s*0.08f, s*0.7f}, color);
        break;
    }

    // ── Misc ────────────────────────────────────────────────────────────
    case IconName::Info: {
        ctx.strokeEllipse(pt(0.5f,0.5f), s*0.35f, s*0.35f, color, t);
        ctx.fillEllipse(pt(0.5f,0.3f), s*0.04f, s*0.04f, color);
        ctx.drawLine(pt(0.5f,0.42f), pt(0.5f,0.72f), color, t*1.2f);
        break;
    }
    case IconName::Warning: {
        ctx.drawLine(pt(0.5f,0.1f), pt(0.1f,0.85f), color, t);
        ctx.drawLine(pt(0.1f,0.85f), pt(0.9f,0.85f), color, t);
        ctx.drawLine(pt(0.9f,0.85f), pt(0.5f,0.1f), color, t);
        ctx.fillEllipse(pt(0.5f,0.72f), s*0.035f, s*0.035f, color);
        ctx.drawLine(pt(0.5f,0.38f), pt(0.5f,0.62f), color, t*1.1f);
        break;
    }
    case IconName::Error: {
        ctx.strokeEllipse(pt(0.5f,0.5f), s*0.35f, s*0.35f, color, t);
        ctx.drawLine(pt(0.32f,0.32f), pt(0.68f,0.68f), color, t);
        ctx.drawLine(pt(0.68f,0.32f), pt(0.32f,0.68f), color, t);
        break;
    }
    case IconName::Star: {
        std::vector<Point2f> pts;
        for (int i = 0; i < 10; ++i) {
            f32 a = -1.5708f + 3.14159f * 2.0f * static_cast<f32>(i) / 10.0f;
            f32 r = (i % 2 == 0) ? 0.4f : 0.18f;
            pts.push_back(pt(0.5f + r * std::cos(a), 0.5f + r * std::sin(a)));
        }
        ctx.fillPolygon(pts, color);
        break;
    }
    case IconName::Heart: {
        // Approximate heart with polyline
        std::vector<Point2f> hp;
        for (int i = 0; i <= 20; ++i) {
            f32 v = static_cast<f32>(i) / 20.0f * 3.14159f * 2.0f;
            f32 hx = 0.5f + 0.35f * (16.0f * std::pow(std::sin(v), 3.0f)) / 16.0f;
            f32 hy = 0.45f - 0.32f * (13.0f * std::cos(v) - 5.0f * std::cos(2*v) - 2.0f * std::cos(3*v) - std::cos(4*v)) / 16.0f;
            hp.push_back(pt(hx, hy));
        }
        ctx.fillPolygon(hp, color);
        break;
    }
    case IconName::Eye: {
        // Eye shape: top arc + bottom arc + circle iris
        std::vector<Point2f> eyeTop, eyeBot;
        for (int i = 0; i <= 12; ++i) {
            f32 u = static_cast<f32>(i) / 12.0f;
            f32 ex = 0.1f + 0.8f * u;
            f32 ey = 0.5f - 0.22f * std::sin(u * 3.14159f);
            eyeTop.push_back(pt(ex, ey));
        }
        for (int i = 0; i <= 12; ++i) {
            f32 u = static_cast<f32>(i) / 12.0f;
            f32 ex = 0.1f + 0.8f * u;
            f32 ey = 0.5f + 0.22f * std::sin(u * 3.14159f);
            eyeBot.push_back(pt(ex, ey));
        }
        ctx.drawPolyline(eyeTop, color, t, false, LineJoin::Round, LineCap::Round);
        ctx.drawPolyline(eyeBot, color, t, false, LineJoin::Round, LineCap::Round);
        ctx.fillEllipse(pt(0.5f,0.5f), s*0.1f, s*0.1f, color);
        break;
    }
    case IconName::EyeOff: {
        // Eye shape + diagonal line
        std::vector<Point2f> eyeTop, eyeBot;
        for (int i = 0; i <= 12; ++i) {
            f32 u = static_cast<f32>(i) / 12.0f;
            f32 ex = 0.1f + 0.8f * u;
            f32 ey = 0.5f - 0.22f * std::sin(u * 3.14159f);
            eyeTop.push_back(pt(ex, ey));
        }
        for (int i = 0; i <= 12; ++i) {
            f32 u = static_cast<f32>(i) / 12.0f;
            f32 ex = 0.1f + 0.8f * u;
            f32 ey = 0.5f + 0.22f * std::sin(u * 3.14159f);
            eyeBot.push_back(pt(ex, ey));
        }
        ctx.drawPolyline(eyeTop, color, t, false, LineJoin::Round, LineCap::Round);
        ctx.drawPolyline(eyeBot, color, t, false, LineJoin::Round, LineCap::Round);
        ctx.fillEllipse(pt(0.5f, 0.5f), s * 0.1f, s * 0.1f, color);
        ctx.drawLine(pt(0.15f, 0.15f), pt(0.85f, 0.85f), color, t * 1.3f);
        break;
    }
    case IconName::Lock: {
        // Padlock body + shackle
        ctx.fillRoundedRect({px(0.2f), py(0.45f), s*0.6f, s*0.45f}, s*0.06f, color);
        std::vector<Point2f> shackle;
        for (int i = 0; i <= 10; ++i) {
            f32 a = 3.14159f + 3.14159f * static_cast<f32>(i) / 10.0f;
            shackle.push_back(pt(0.5f + 0.17f*std::cos(a), 0.45f + 0.2f*std::sin(a)));
        }
        ctx.drawPolyline(shackle, color, t*1.3f, false, LineJoin::Round, LineCap::Round);
        break;
    }
    case IconName::Unlock: {
        // Open padlock — shackle shifted up-right
        ctx.fillRoundedRect({px(0.2f), py(0.45f), s*0.6f, s*0.45f}, s*0.06f, color);
        ctx.drawLine(pt(0.33f,0.45f), pt(0.33f,0.32f), color, t*1.3f);
        std::vector<Point2f> shackle;
        for (int i = 0; i <= 8; ++i) {
            f32 a = 3.14159f + 3.14159f * static_cast<f32>(i) / 10.0f;
            shackle.push_back(pt(0.5f + 0.17f*std::cos(a), 0.32f + 0.17f*std::sin(a)));
        }
        ctx.drawPolyline(shackle, color, t*1.3f, false, LineJoin::Round, LineCap::Round);
        break;
    }
    case IconName::User: {
        // Head circle + body arc
        ctx.strokeEllipse(pt(0.5f,0.3f), s*0.15f, s*0.15f, color, t);
        std::vector<Point2f> body;
        for (int i = 0; i <= 10; ++i) {
            f32 a = 3.14159f + 3.14159f * static_cast<f32>(i) / 10.0f;
            body.push_back(pt(0.5f + 0.3f*std::cos(a), 0.9f + 0.25f*std::sin(a)));
        }
        ctx.drawPolyline(body, color, t, false, LineJoin::Round, LineCap::Round);
        break;
    }
    case IconName::Download: {
        // Arrow down into tray
        ctx.drawLine(pt(0.5f,0.12f), pt(0.5f,0.6f), color, t);
        ctx.drawLine(pt(0.5f,0.6f), pt(0.3f,0.42f), color, t);
        ctx.drawLine(pt(0.5f,0.6f), pt(0.7f,0.42f), color, t);
        ctx.drawLine(pt(0.15f,0.75f), pt(0.15f,0.88f), color, t);
        ctx.drawLine(pt(0.15f,0.88f), pt(0.85f,0.88f), color, t);
        ctx.drawLine(pt(0.85f,0.88f), pt(0.85f,0.75f), color, t);
        break;
    }
    case IconName::Upload: {
        // Arrow up from tray
        ctx.drawLine(pt(0.5f,0.6f), pt(0.5f,0.12f), color, t);
        ctx.drawLine(pt(0.5f,0.12f), pt(0.3f,0.3f), color, t);
        ctx.drawLine(pt(0.5f,0.12f), pt(0.7f,0.3f), color, t);
        ctx.drawLine(pt(0.15f,0.75f), pt(0.15f,0.88f), color, t);
        ctx.drawLine(pt(0.15f,0.88f), pt(0.85f,0.88f), color, t);
        ctx.drawLine(pt(0.85f,0.88f), pt(0.85f,0.75f), color, t);
        break;
    }

    default:
        break;
    }
}

// ── IconElement ─────────────────────────────────────────────────────────

IconElement::IconElement(IconName name) {
    seticon(name);
}

Size2f IconElement::measureOverride(Size2f /*availableSize*/) {
    return {size(), size()};
}

void IconElement::onRender(RenderContext& ctx) {
    Icon::draw(ctx, icon(), {0, 0, bounds().width, bounds().height}, color(), strokeWeight());
}

} // namespace gut


// === Toolbar implementation ==================================================

namespace gut {

Toolbar::Toolbar() {
    setfocusable(true);
}

void Toolbar::addButton(IconName icon, String label, std::function<void()> action, String tooltip) {
    m_items.push_back({ToolbarItemType::Button, icon, std::move(label), std::move(tooltip),
                        std::move(action), nullptr, false, true});
    invalidateLayout();
    invalidateRender();
}

void Toolbar::addButton(IconName icon, std::function<void()> action, String tooltip) {
    addButton(icon, "", std::move(action), std::move(tooltip));
}

void Toolbar::addButton(String label, std::function<void()> action, String tooltip) {
    addButton(IconName::None, std::move(label), std::move(action), std::move(tooltip));
}

void Toolbar::addToggle(IconName icon, String label, bool initialState,
                         std::function<void(bool)> onToggled, String tooltip) {
    m_items.push_back({ToolbarItemType::Toggle, icon, std::move(label), std::move(tooltip),
                        nullptr, std::move(onToggled), initialState, true});
    invalidateLayout();
    invalidateRender();
}

void Toolbar::addSeparator() {
    m_items.push_back({ToolbarItemType::Separator, IconName::None, "", "", nullptr, nullptr, false, true});
    invalidateLayout();
    invalidateRender();
}

void Toolbar::setToggled(usize index, bool value) {
    if (index < m_items.size() && m_items[index].type == ToolbarItemType::Toggle) {
        m_items[index].toggled = value;
        invalidateRender();
    }
}

void Toolbar::setItemEnabled(usize index, bool enabled) {
    if (index < m_items.size()) {
        m_items[index].enabled = enabled;
        invalidateRender();
    }
}

f32 Toolbar::itemWidth(const Item& item) const {
    if (item.type == ToolbarItemType::Separator) return separatorWidth() + spacing() * 2.0f;

    f32 w = itemPadding() * 2.0f;
    if (item.icon != IconName::None) w += iconSize();
    if (!item.label.empty()) {
        f32 labelW = static_cast<f32>(item.label.size()) * fontSize() * 0.65f; // approximate
        if (item.icon != IconName::None) w += spacing(); // gap between icon and label
        w += labelW;
    }
    // Minimum width = height (square for icon-only buttons)
    if (item.icon != IconName::None && item.label.empty()) {
        w = std::max(w, itemHeight());
    }
    return w;
}

void Toolbar::computeLayout() {
    m_layout.clear();
    f32 x = spacing();
    for (usize i = 0; i < m_items.size(); ++i) {
        f32 w = itemWidth(m_items[i]);
        m_layout.push_back({{x, 0, w, itemHeight()}, i});
        x += w + spacing();
    }
}

isize Toolbar::itemIndexAtX(f32 x) const {
    for (auto& lay : m_layout) {
        if (x >= lay.rect.x && x < lay.rect.x + lay.rect.width) {
            auto& item = m_items[lay.itemIndex];
            if (item.type == ToolbarItemType::Separator) return -1;
            return static_cast<isize>(lay.itemIndex);
        }
    }
    return -1;
}

Size2f Toolbar::measureOverride(Size2f availableSize) {
    computeLayout();
    f32 totalW = 0;
    if (!m_layout.empty()) {
        auto& last = m_layout.back();
        totalW = last.rect.x + last.rect.width + spacing();
    }
    return {std::max(totalW, availableSize.width), itemHeight()};
}

void Toolbar::onRender(RenderContext& ctx) {
    computeLayout();

    // Background
    ctx.fillRect({0, 0, bounds().width, bounds().height}, toolbarBackground());
    // Bottom border
    ctx.fillRect({0, bounds().height - 1, bounds().width, 1}, borderColor());

    Font* font = context() ? context()->defaultFont() : nullptr;
    Ref<FontFace> face;
    if (font) face = font->getFace(fontSize());

    for (auto& lay : m_layout) {
        const auto& item = m_items[lay.itemIndex];
        Rectf r = lay.rect;
        isize idx = static_cast<isize>(lay.itemIndex);

        if (item.type == ToolbarItemType::Separator) {
            f32 sx = r.x + r.width * 0.5f;
            f32 topPad = r.height * 0.2f;
            ctx.drawLine({sx, topPad}, {sx, r.height - topPad}, separatorColor(), separatorWidth());
            continue;
        }

        // Background highlight
        bool hovered = (idx == m_hoveredItem && item.enabled);
        bool pressed = (idx == m_pressedItem && item.enabled);
        bool toggled = (item.type == ToolbarItemType::Toggle && item.toggled);

        if (pressed) {
            ctx.fillRoundedRect(r, 4.0f, itemPressedBackground());
        } else if (toggled) {
            ctx.fillRoundedRect(r, 4.0f, itemToggledBackground());
        } else if (hovered) {
            ctx.fillRoundedRect(r, 4.0f, itemHoverBackground());
        }

        Color fg = item.enabled ? itemForeground() : itemDisabledForeground();

        // Position icon and label inside item rect
        f32 contentX = r.x + itemPadding();

        if (item.icon != IconName::None) {
            f32 icoS = iconSize();
            f32 icoY = r.y + (r.height - icoS) * 0.5f;
            // If no label, centre icon
            if (item.label.empty()) {
                f32 icoX = r.x + (r.width - icoS) * 0.5f;
                Icon::draw(ctx, item.icon, {icoX, icoY, icoS, icoS}, fg);
            } else {
                Icon::draw(ctx, item.icon, {contentX, icoY, icoS, icoS}, fg);
                contentX += icoS + spacing();
            }
        }

        if (!item.label.empty() && face) {
            f32 ty = r.y + (r.height - face->lineHeight()) * 0.5f + face->ascender();
            ctx.drawText(face.get(), item.label, {contentX, ty}, fg);
        }
    }
}

bool Toolbar::onMouseEvent(const MouseEvent& event) {
    switch (event.type) {
        case MouseEventType::Move: {
            isize idx = itemIndexAtX(event.position.x);
            if (idx != m_hoveredItem) {
                m_hoveredItem = idx;
                invalidateRender();
            }
            return false;
        }
        case MouseEventType::ButtonDown: {
            if (event.button == MouseButton::Left) {
                isize idx = itemIndexAtX(event.position.x);
                if (idx >= 0 && m_items[static_cast<usize>(idx)].enabled) {
                    m_pressedItem = idx;
                    invalidateRender();
                }
                return true;
            }
            return false;
        }
        case MouseEventType::ButtonUp: {
            if (event.button == MouseButton::Left && m_pressedItem >= 0) {
                isize idx = itemIndexAtX(event.position.x);
                if (idx == m_pressedItem) {
                    auto& item = m_items[static_cast<usize>(idx)];
                    if (item.enabled) {
                        if (item.type == ToolbarItemType::Toggle) {
                            item.toggled = !item.toggled;
                            if (item.onToggled) item.onToggled(item.toggled);
                        } else if (item.onClick) {
                            item.onClick();
                        }
                    }
                }
                m_pressedItem = -1;
                invalidateRender();
                return true;
            }
            return false;
        }
        default:
            return false;
    }
}

void Toolbar::onMouseEnter() {
    Element::onMouseEnter();
}

void Toolbar::onMouseLeave() {
    if (m_hoveredItem != -1) {
        m_hoveredItem = -1;
        invalidateRender();
    }
    Element::onMouseLeave();
}

} // namespace gut


// --- elements/Image.cpp ---

#include <cmath>
#include <algorithm>

namespace gut {

Image::Image(Ref<Texture> tex) : m_texture(std::move(tex)) {}

void Image::setTexture(Ref<Texture> tex) {
    m_texture = std::move(tex);
    invalidateLayout();
    invalidateRender();
}

void Image::setSourceRect(Rectf rect) {
    m_sourceRect = rect;
    m_hasSourceRect = true;
    invalidateRender();
}

void Image::clearSourceRect() {
    m_hasSourceRect = false;
    invalidateRender();
}

Size2f Image::measureOverride(Size2f availableSize) {
    if (!m_texture) {
        return {0, 0};
    }
    
    Size2f imageSize = m_hasSourceRect 
        ? m_sourceRect.size() 
        : m_texture->size();
    
    switch (stretch()) {
        case Stretch::None:
            return imageSize;
            
        case Stretch::Fill:
            return availableSize;
            
        case Stretch::Uniform: {
            f32 scaleX = availableSize.width / imageSize.width;
            f32 scaleY = availableSize.height / imageSize.height;
            f32 scale = std::min(scaleX, scaleY);
            return {imageSize.width * scale, imageSize.height * scale};
        }
        
        case Stretch::UniformToFill: {
            f32 scaleX = availableSize.width / imageSize.width;
            f32 scaleY = availableSize.height / imageSize.height;
            f32 scale = std::max(scaleX, scaleY);
            return {imageSize.width * scale, imageSize.height * scale};
        }
    }
    
    return imageSize;
}

void Image::onRender(RenderContext& ctx) {
    if (!m_texture) {
        return;
    }
    
    Size2f imageSize = m_hasSourceRect 
        ? m_sourceRect.size() 
        : m_texture->size();
    
    Rectf destRect = {0, 0, bounds().width, bounds().height};
    
    // Apply stretch mode
    switch (stretch()) {
        case Stretch::None: {
            // Center the image at natural size
            f32 x = 0, y = 0;
            
            switch (horizontalImageAlignment()) {
                case HorizontalAlignment::Center:
                    x = (bounds().width - imageSize.width) / 2;
                    break;
                case HorizontalAlignment::Right:
                    x = bounds().width - imageSize.width;
                    break;
                default:
                    break;
            }
            
            switch (verticalImageAlignment()) {
                case VerticalAlignment::Center:
                    y = (bounds().height - imageSize.height) / 2;
                    break;
                case VerticalAlignment::Bottom:
                    y = bounds().height - imageSize.height;
                    break;
                default:
                    break;
            }
            
            destRect = {x, y, imageSize.width, imageSize.height};
            break;
        }
        
        case Stretch::Fill:
            // Fill entire bounds, may distort
            break;
            
        case Stretch::Uniform: {
            f32 scaleX = bounds().width / imageSize.width;
            f32 scaleY = bounds().height / imageSize.height;
            f32 scale = std::min(scaleX, scaleY);
            
            f32 scaledWidth = imageSize.width * scale;
            f32 scaledHeight = imageSize.height * scale;
            
            f32 x = (bounds().width - scaledWidth) / 2;
            f32 y = (bounds().height - scaledHeight) / 2;
            
            destRect = {x, y, scaledWidth, scaledHeight};
            break;
        }
        
        case Stretch::UniformToFill: {
            f32 scaleX = bounds().width / imageSize.width;
            f32 scaleY = bounds().height / imageSize.height;
            f32 scale = std::max(scaleX, scaleY);
            
            f32 scaledWidth = imageSize.width * scale;
            f32 scaledHeight = imageSize.height * scale;
            
            f32 x = (bounds().width - scaledWidth) / 2;
            f32 y = (bounds().height - scaledHeight) / 2;
            
            destRect = {x, y, scaledWidth, scaledHeight};
            break;
        }
        
        case Stretch::NineSlice:
            // destRect is the full bounds; drawImageNineSlice handles the slicing
            break;
    }
    
    if (stretch() == Stretch::NineSlice) {
        if (m_hasSourceRect) {
            ctx.drawImageNineSlice(*m_texture, m_sourceRect, sliceBorders(), destRect, tint());
        } else {
            ctx.drawImageNineSlice(*m_texture, sliceBorders(), destRect, tint());
        }
    } else if (m_hasSourceRect) {
        ctx.drawImage(*m_texture, m_sourceRect, destRect, tint());
    } else {
        ctx.drawImage(*m_texture, destRect, tint());
    }
}

} // namespace gut


// --- elements/ScrollViewer.cpp ---

#include <algorithm>

namespace gut {

void ScrollViewer::scrollTo(f32 hOffset, f32 vOffset) {
    sethorizontalOffset(std::clamp(hOffset, 0.0f, std::max(0.0f, m_scrollableWidth - m_viewportWidth)));
    setverticalOffset(std::clamp(vOffset, 0.0f, std::max(0.0f, m_scrollableHeight - m_viewportHeight)));
}

void ScrollViewer::scrollBy(f32 deltaX, f32 deltaY) {
    scrollTo(horizontalOffset() + deltaX, verticalOffset() + deltaY);
}

void ScrollViewer::scrollIntoView(Element* element) {
    if (!element) return;
    
    // Find element bounds relative to this ScrollViewer
    Rectf elementBounds = element->bounds();
    Element* current = element->parent();
    while (current && current != this) {
        elementBounds.x += current->bounds().x;
        elementBounds.y += current->bounds().y;
        current = current->parent();
    }
    
    // Adjust scroll to make element visible
    if (elementBounds.x < horizontalOffset()) {
        scrollTo(elementBounds.x, verticalOffset());
    } else if (elementBounds.right() > horizontalOffset() + m_viewportWidth) {
        scrollTo(elementBounds.right() - m_viewportWidth, verticalOffset());
    }
    
    if (elementBounds.y < verticalOffset()) {
        scrollTo(horizontalOffset(), elementBounds.y);
    } else if (elementBounds.bottom() > verticalOffset() + m_viewportHeight) {
        scrollTo(horizontalOffset(), elementBounds.bottom() - m_viewportHeight);
    }
}

void ScrollViewer::scrollToTop() {
    scrollTo(horizontalOffset(), 0);
}

void ScrollViewer::scrollToBottom() {
    scrollTo(horizontalOffset(), m_scrollableHeight - m_viewportHeight);
    m_pendingScrollToBottom = true;
}

Size2f ScrollViewer::measureOverride(Size2f availableSize) {
    if (m_children.empty()) {
        return {0, 0};
    }
    
    // Measure content with infinite size in scrollable directions
    Size2f contentAvailable = availableSize;
    
    if (horizontalScrollBarVisibility() != ScrollBarVisibility::Disabled) {
        contentAvailable.width = INFINITY;
    }
    if (verticalScrollBarVisibility() != ScrollBarVisibility::Disabled) {
        contentAvailable.height = INFINITY;
    }
    
    m_children[0]->measure(contentAvailable);
    Size2f contentSize = m_children[0]->desiredSize();
    
    m_scrollableWidth = contentSize.width;
    m_scrollableHeight = contentSize.height;
    
    // If vertical scrollbar will be needed and horizontal is disabled,
    // re-measure with reduced width to account for scrollbar space
    if (horizontalScrollBarVisibility() == ScrollBarVisibility::Disabled &&
        verticalScrollBarVisibility() != ScrollBarVisibility::Disabled &&
        m_scrollableHeight > availableSize.height) {
        contentAvailable.width = availableSize.width - scrollBarWidth();
        m_children[0]->measure(contentAvailable);
        contentSize = m_children[0]->desiredSize();
        m_scrollableWidth = contentSize.width;
        m_scrollableHeight = contentSize.height;
    }
    
    // Return available size (we take whatever space is given)
    return availableSize;
}

Size2f ScrollViewer::arrangeOverride(Size2f finalSize) {
    if (m_children.empty()) {
        return finalSize;
    }
    
    // Account for scrollbars
    m_viewportWidth = finalSize.width;
    m_viewportHeight = finalSize.height;
    
    bool needsHScroll = needsHorizontalScrollBar();
    bool needsVScroll = needsVerticalScrollBar();
    
    if (needsVScroll) {
        m_viewportWidth -= scrollBarWidth();
    }
    if (needsHScroll) {
        m_viewportHeight -= scrollBarWidth();
    }
    
    // Re-check after adjusting for scrollbars
    needsHScroll = needsHorizontalScrollBar();
    needsVScroll = needsVerticalScrollBar();
    
    // Clamp scroll offsets (or apply pending scrollToBottom with up-to-date measurements)
    if (m_pendingScrollToBottom) {
        scrollTo(horizontalOffset(), m_scrollableHeight - m_viewportHeight);
        m_pendingScrollToBottom = false;
    } else {
        scrollTo(horizontalOffset(), verticalOffset());
    }
    
    // Arrange content at scroll offset
    Size2f contentSize = {
        std::max(m_scrollableWidth, m_viewportWidth),
        std::max(m_scrollableHeight, m_viewportHeight)
    };
    
    m_children[0]->arrange({-horizontalOffset(), -verticalOffset(), contentSize.width, contentSize.height});
    
    return finalSize;
}

void ScrollViewer::onRender(RenderContext& ctx) {
    // Draw background
    if (background().a > 0) {
        ctx.fillRect({0, 0, bounds().width, bounds().height}, background());
    }
    
    // Clip to viewport and render content
    ctx.pushClip({0, 0, m_viewportWidth, m_viewportHeight});
    renderChildren(ctx);
    ctx.popClip();
    
    // Draw scrollbars
    bool needsVScroll = needsVerticalScrollBar();
    bool needsHScroll = needsHorizontalScrollBar();
    
    if (needsVScroll) {
        f32 trackX = bounds().width - scrollBarWidth();
        f32 arrowH = scrollArrowSize();
        f32 trackHeight = m_viewportHeight;
        
        // Track background
        ctx.fillRect({trackX, 0, scrollBarWidth(), trackHeight}, scrollBarBackground());
        
        // Up arrow button
        if (arrowH > 0) {
            Rectf ar = {trackX, 0, scrollBarWidth(), arrowH};
            Color arC = m_upArrowPressed ? scrollBarThumbDrag()
                      : m_upArrowHovered ? scrollBarThumbHover()
                      : scrollBarThumb();
            ctx.fillRect(ar, arC);
            // Draw up triangle
            f32 cx = trackX + scrollBarWidth() * 0.5f;
            f32 cy = arrowH * 0.5f;
            f32 sz = 3.0f;
            ctx.fillTriangle({cx, cy - sz}, {cx - sz, cy + sz}, {cx + sz, cy + sz},
                             Color::fromHex(0x404040));
        }
        
        // Down arrow button
        if (arrowH > 0) {
            Rectf ar = {trackX, trackHeight - arrowH, scrollBarWidth(), arrowH};
            Color arC = m_downArrowPressed ? scrollBarThumbDrag()
                      : m_downArrowHovered ? scrollBarThumbHover()
                      : scrollBarThumb();
            ctx.fillRect(ar, arC);
            // Draw down triangle
            f32 cx = trackX + scrollBarWidth() * 0.5f;
            f32 cy = trackHeight - arrowH * 0.5f;
            f32 sz = 3.0f;
            ctx.fillTriangle({cx, cy + sz}, {cx - sz, cy - sz}, {cx + sz, cy - sz},
                             Color::fromHex(0x404040));
        }
        
        // Thumb
        Rectf thumb = verticalThumbRect();
        if (thumb.height > 0) {
            Color thumbColor = m_draggingVertical ? scrollBarThumbDrag()
                             : m_verticalThumbHovered ? scrollBarThumbHover()
                             : scrollBarThumb();
            ctx.fillRoundedRect({thumb.x + 2, thumb.y + 1, thumb.width - 4, thumb.height - 2},
                                3, thumbColor);
        }
    }
    
    if (needsHScroll) {
        f32 trackY = bounds().height - scrollBarWidth();
        f32 trackWidth = m_viewportWidth;
        
        // Track
        ctx.fillRect({0, trackY, trackWidth, scrollBarWidth()}, scrollBarBackground());
        
        // Thumb
        f32 thumbWidth = (m_viewportWidth / m_scrollableWidth) * trackWidth;
        thumbWidth = std::max(thumbWidth, 20.0f);
        f32 thumbX = (horizontalOffset() / (m_scrollableWidth - m_viewportWidth)) * (trackWidth - thumbWidth);
        
        Color thumbColor = m_horizontalThumbHovered ? scrollBarThumbHover() : scrollBarThumb();
        ctx.fillRoundedRect({thumbX + 2, trackY + 2, thumbWidth - 4, scrollBarWidth() - 4}, 3, thumbColor);
    }
}

Rectf ScrollViewer::verticalTrackRect() const {
    f32 trackX = bounds().width - scrollBarWidth();
    f32 arrowH = scrollArrowSize();
    return {trackX, arrowH, scrollBarWidth(), m_viewportHeight - arrowH * 2};
}

Rectf ScrollViewer::verticalThumbRect() const {
    f32 trackX = bounds().width - scrollBarWidth();
    f32 arrowH = scrollArrowSize();
    f32 innerHeight = m_viewportHeight - arrowH * 2;
    f32 maxScroll = m_scrollableHeight - m_viewportHeight;
    if (maxScroll <= 0 || innerHeight <= 0) return {trackX, arrowH, scrollBarWidth(), 0};
    
    f32 thumbHeight = std::max((m_viewportHeight / m_scrollableHeight) * innerHeight, 20.0f);
    f32 thumbY = arrowH + (verticalOffset() / maxScroll) * (innerHeight - thumbHeight);
    return {trackX, thumbY, scrollBarWidth(), thumbHeight};
}

Rectf ScrollViewer::verticalUpArrowRect() const {
    f32 trackX = bounds().width - scrollBarWidth();
    return {trackX, 0, scrollBarWidth(), scrollArrowSize()};
}

Rectf ScrollViewer::verticalDownArrowRect() const {
    f32 trackX = bounds().width - scrollBarWidth();
    return {trackX, m_viewportHeight - scrollArrowSize(), scrollBarWidth(), scrollArrowSize()};
}

bool ScrollViewer::onMouseEvent(const MouseEvent& event) {
    if (event.type == MouseEventType::Wheel) {
        scrollBy(-event.delta.x, -event.delta.y);
        return true;
    }
    
    bool needsVScroll = needsVerticalScrollBar();
    Point2f pos = event.position;
    
    if (needsVScroll) {
        Rectf thumb = verticalThumbRect();
        Rectf upArr = verticalUpArrowRect();
        Rectf downArr = verticalDownArrowRect();
        Rectf track = verticalTrackRect();
        
        bool inThumb = pos.x >= thumb.x && pos.x < thumb.x + thumb.width &&
                       pos.y >= thumb.y && pos.y < thumb.y + thumb.height;
        bool inUpArr = pos.x >= upArr.x && pos.x < upArr.x + upArr.width &&
                       pos.y >= upArr.y && pos.y < upArr.y + upArr.height;
        bool inDownArr = pos.x >= downArr.x && pos.x < downArr.x + downArr.width &&
                         pos.y >= downArr.y && pos.y < downArr.y + downArr.height;
        bool inTrack = pos.x >= track.x && pos.x < track.x + track.width &&
                       pos.y >= track.y && pos.y < track.y + track.height;
        
        switch (event.type) {
            case MouseEventType::ButtonDown:
                if (event.button == MouseButton::Left) {
                    if (inThumb) {
                        // Start dragging thumb
                        m_draggingVertical = true;
                        m_dragStartOffset = verticalOffset();
                        m_dragStartMouse = pos.y;
                        if (context()) context()->inputManager().captureMouse(this);
                        return true;
                    } else if (inUpArr) {
                        m_upArrowPressed = true;
                        scrollBy(0, -30);
                        invalidateRender();
                        return true;
                    } else if (inDownArr) {
                        m_downArrowPressed = true;
                        scrollBy(0, 30);
                        invalidateRender();
                        return true;
                    } else if (inTrack && !inThumb) {
                        // Click in track — page scroll toward click
                        if (pos.y < thumb.y) {
                            scrollBy(0, -m_viewportHeight * 0.8f);
                        } else {
                            scrollBy(0, m_viewportHeight * 0.8f);
                        }
                        return true;
                    }
                }
                break;
                
            case MouseEventType::ButtonUp:
                if (event.button == MouseButton::Left) {
                    if (m_draggingVertical) {
                        m_draggingVertical = false;
                        if (context()) context()->inputManager().releaseMouse();
                        invalidateRender();
                        return true;
                    }
                    if (m_upArrowPressed || m_downArrowPressed) {
                        m_upArrowPressed = false;
                        m_downArrowPressed = false;
                        invalidateRender();
                        return true;
                    }
                }
                break;
                
            case MouseEventType::Move: {
                if (m_draggingVertical) {
                    // Map mouse delta to scroll delta
                    f32 arrowH = scrollArrowSize();
                    f32 innerHeight = m_viewportHeight - arrowH * 2;
                    f32 thumbHeight = std::max((m_viewportHeight / m_scrollableHeight) * innerHeight, 20.0f);
                    f32 trackRange = innerHeight - thumbHeight;
                    if (trackRange > 0) {
                        f32 mouseDelta = pos.y - m_dragStartMouse;
                        f32 scrollRange = m_scrollableHeight - m_viewportHeight;
                        f32 newOffset = m_dragStartOffset + (mouseDelta / trackRange) * scrollRange;
                        scrollTo(horizontalOffset(), newOffset);
                    }
                    invalidateRender();
                    return true;
                }
                // Hover detection
                bool newThumbHover = inThumb;
                bool newUpHover = inUpArr;
                bool newDownHover = inDownArr;
                if (newThumbHover != m_verticalThumbHovered ||
                    newUpHover != m_upArrowHovered ||
                    newDownHover != m_downArrowHovered) {
                    m_verticalThumbHovered = newThumbHover;
                    m_upArrowHovered = newUpHover;
                    m_downArrowHovered = newDownHover;
                    invalidateRender();
                }
                break;
            }
                
            default:
                break;
        }
    }
    
    return false;
}

void ScrollViewer::onMouseLeave() {
    if (m_verticalThumbHovered || m_upArrowHovered || m_downArrowHovered) {
        m_verticalThumbHovered = false;
        m_upArrowHovered = false;
        m_downArrowHovered = false;
        invalidateRender();
    }
}

void ScrollViewer::updateScrollBars() {
    // Called when content size or viewport changes
    scrollTo(horizontalOffset(), verticalOffset());
}

bool ScrollViewer::needsHorizontalScrollBar() const {
    switch (horizontalScrollBarVisibility()) {
        case ScrollBarVisibility::Disabled:
        case ScrollBarVisibility::Hidden:
            return false;
        case ScrollBarVisibility::Visible:
            return true;
        case ScrollBarVisibility::Auto:
            return m_scrollableWidth > m_viewportWidth;
    }
    return false;
}

bool ScrollViewer::needsVerticalScrollBar() const {
    switch (verticalScrollBarVisibility()) {
        case ScrollBarVisibility::Disabled:
        case ScrollBarVisibility::Hidden:
            return false;
        case ScrollBarVisibility::Visible:
            return true;
        case ScrollBarVisibility::Auto:
            return m_scrollableHeight > m_viewportHeight;
    }
    return false;
}

} // namespace gut


// --- elements/TextBox.cpp ---


namespace gut {

TextBox::TextBox() {
    setfocusable(true);
    setpadding(Thickness{6, 4, 6, 4});
    setcursor(CursorType::IBeam);
}

TextBox::TextBox(String initialText) {
    settext(std::move(initialText));
    m_caretPos = static_cast<i32>(text().size());
    setfocusable(true);
    setpadding(Thickness{6, 4, 6, 4});
    setcursor(CursorType::IBeam);
}

void TextBox::setCaretPosition(i32 pos) {
    m_caretPos = std::clamp(pos, 0, static_cast<i32>(text().size()));
    m_caretBlinkTimer = 0.0f;
    m_caretVisible = true;
}

i32 TextBox::selectionStart() const {
    if (m_selAnchor < 0) return m_caretPos;
    return std::min(m_selAnchor, m_caretPos);
}

i32 TextBox::selectionEnd() const {
    if (m_selAnchor < 0) return m_caretPos;
    return std::max(m_selAnchor, m_caretPos);
}

String TextBox::selectedText() const {
    if (!hasSelection()) return "";
    return text().substr(selectionStart(), selectionEnd() - selectionStart());
}

void TextBox::select(i32 start, i32 end) {
    i32 len = static_cast<i32>(text().size());
    m_selAnchor = std::clamp(start, 0, len);
    m_caretPos = std::clamp(end, 0, len);
}

void TextBox::selectAll() {
    m_selAnchor = 0;
    m_caretPos = static_cast<i32>(text().size());
}

void TextBox::clearSelection() {
    m_selAnchor = -1;
}

void TextBox::insertText(const String& str) {
    if (readOnly()) return;
    String t = text();
    if (hasSelection()) {
        i32 s = selectionStart();
        i32 e = selectionEnd();
        t.erase(s, e - s);
        m_caretPos = s;
    }
    m_selAnchor = -1;
    t.insert(m_caretPos, str);
    m_caretPos += static_cast<i32>(str.size());
    settext(std::move(t));
    scrollToCaret();
    textChanged.emit(text());
    if (m_onTextChanged) m_onTextChanged(text());
    invalidateLayout();
    invalidateRender();
}

void TextBox::deleteText(i32 count) {
    if (readOnly()) return;
    String t = text();
    if (hasSelection()) {
        i32 s = selectionStart();
        i32 e = selectionEnd();
        t.erase(s, e - s);
        m_caretPos = s;
    } else if (count < 0) {
        // Backspace
        i32 deleteCount = std::min(-count, m_caretPos);
        if (deleteCount > 0) {
            t.erase(m_caretPos - deleteCount, deleteCount);
            m_caretPos -= deleteCount;
        }
    } else if (count > 0) {
        // Delete forward
        i32 deleteCount = std::min(count, static_cast<i32>(t.size()) - m_caretPos);
        if (deleteCount > 0) {
            t.erase(m_caretPos, deleteCount);
        }
    }
    m_selAnchor = -1;
    settext(std::move(t));
    scrollToCaret();
    textChanged.emit(text());
    if (m_onTextChanged) m_onTextChanged(text());
    invalidateLayout();
    invalidateRender();
}

void TextBox::moveCaret(i32 newPos, bool selecting) {
    i32 len = static_cast<i32>(text().size());
    newPos = std::clamp(newPos, 0, len);
    if (selecting) {
        if (m_selAnchor < 0) m_selAnchor = m_caretPos;
    } else {
        m_selAnchor = -1;
    }
    m_caretPos = newPos;
    m_caretBlinkTimer = 0.0f;
    m_caretVisible = true;
    scrollToCaret();
    invalidateRender();
}

String TextBox::displayText() const {
    const String& t = text();
    if (!isPassword() || t.empty()) return t;
    // Use ASCII bullet substitute (renderer only supports ASCII 32-126)
    return String(t.size(), '*');
}

i32 TextBox::hitTestCaret(f32 localX) const {
    f32 textX = localX - padding().left + m_scrollOffset;
    const String& t = displayText();
    if (t.empty()) return 0;

    // Use font system to measure character by character
    FontFace* face = nullptr;
    if (context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto faceRef = font->getFace(fontSize());
            face = faceRef.get();
        }
    }

    // Iterate by logical character count, measuring the display glyph.
    i32 logicalLen = static_cast<i32>(text().size());
    f32 x = 0.0f;
    for (i32 i = 0; i < logicalLen; ++i) {
        f32 charW = 0.0f;
        if (face) {
            char32_t ch = isPassword() ? U'*' : static_cast<char32_t>(text()[i]);
            auto g = face->glyph(static_cast<u32>(ch));
            charW = g ? g->advance : fontSize() * 0.5f;
        } else {
            charW = fontSize() * 0.5f;
        }
        if (textX < x + charW * 0.5f) return i;
        x += charW;
    }
    return logicalLen;
}

f32 TextBox::measureSubstring(i32 pos) const {
    if (pos <= 0) return 0.0f;
    String dt = displayText();
    String sub = dt.substr(0, std::min(pos, static_cast<i32>(dt.size())));

    if (context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(fontSize());
            if (face) {
                return face->measureWidth(sub);
            }
        }
    }
    return static_cast<f32>(sub.size()) * fontSize() * 0.5f;
}

void TextBox::scrollToCaret() {
    f32 caretX = measureSubstring(m_caretPos);
    f32 pad = padding().left + padding().right;
    f32 viewW = bounds().width > 0 ? bounds().width - pad : 200.0f;

    if (caretX - m_scrollOffset > viewW) {
        m_scrollOffset = caretX - viewW;
    }
    if (caretX - m_scrollOffset < 0) {
        m_scrollOffset = caretX;
    }
    if (m_scrollOffset < 0) m_scrollOffset = 0;
}

i32 TextBox::wordBoundaryLeft(i32 pos) const {
    const String& t = text();
    if (pos <= 0) return 0;
    i32 i = pos - 1;
    // Skip non-alphanumeric
    while (i > 0 && !std::isalnum(static_cast<unsigned char>(t[i]))) --i;
    // Skip alphanumeric
    while (i > 0 && std::isalnum(static_cast<unsigned char>(t[i - 1]))) --i;
    return i;
}

i32 TextBox::wordBoundaryRight(i32 pos) const {
    const String& t = text();
    i32 len = static_cast<i32>(t.size());
    if (pos >= len) return len;
    i32 i = pos;
    // Skip alphanumeric
    while (i < len && std::isalnum(static_cast<unsigned char>(t[i]))) ++i;
    // Skip non-alphanumeric
    while (i < len && !std::isalnum(static_cast<unsigned char>(t[i]))) ++i;
    return i;
}

Size2f TextBox::measureOverride(Size2f availableSize) {
    const auto& pad = padding();
    f32 textH = fontSize() * 1.2f;
    if (context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto face = font->getFace(fontSize());
            if (face) textH = face->lineHeight();
        }
    }
    // Width: use available or a sensible default
    f32 w = (width() > 0) ? width() : std::min(availableSize.width, 200.0f);
    f32 h = textH + pad.verticalSum() + borderWidth() * 2;
    return {w, h};
}

void TextBox::onRender(RenderContext& ctx) {
    Rectf rect = {0, 0, bounds().width, bounds().height};
    const auto& pad = padding();

    // Background
    Color bg = background();
    if (cornerRadius() > 0) {
        ctx.fillRoundedRect(rect, cornerRadius(), bg);
    } else {
        ctx.fillRect(rect, bg);
    }

    // Border
    Color bc = isFocused() ? focusBorderColor() : borderColor();
    f32 bw = isFocused() ? std::max(borderWidth(), 2.0f) : borderWidth();
    if (bw > 0) {
        if (cornerRadius() > 0) {
            ctx.strokeRoundedRect(rect, cornerRadius(), bc, bw);
        } else {
            ctx.strokeRect(rect, bc, bw);
        }
    }

    // Clip text area
    Rectf textClip = {pad.left, pad.top,
                      bounds().width - pad.horizontalSum(),
                      bounds().height - pad.verticalSum()};
    ctx.pushClip(textClip, std::max(cornerRadius() - 2.0f, 0.0f));

    const String& t = text();
    String dt = displayText();
    bool showPlaceholder = t.empty() && !placeholder().empty();

    FontFace* face = nullptr;
    if (context()) {
        Font* font = context()->defaultFont();
        if (font) {
            auto faceRef = font->getFace(fontSize());
            face = faceRef.get();
        }
    }

    f32 baseline = pad.top;
    if (face) {
        baseline += face->ascender();
    } else {
        baseline += fontSize();
    }

    // Draw selection highlight
    if (isFocused() && hasSelection() && face) {
        i32 s = selectionStart();
        i32 e = selectionEnd();
        f32 sx = pad.left + measureSubstring(s) - m_scrollOffset;
        f32 ex = pad.left + measureSubstring(e) - m_scrollOffset;
        Rectf selRect = {sx, pad.top, ex - sx, bounds().height - pad.verticalSum()};
        ctx.fillRect(selRect, selectionColor());
    }

    // Draw text or placeholder
    if (face) {
        if (showPlaceholder) {
            ctx.drawText(face, placeholder(), {pad.left, baseline}, placeholderColor());
        } else if (!t.empty()) {
            ctx.drawText(face, dt, {pad.left - m_scrollOffset, baseline}, foreground());
        }
    }

    // Draw caret
    if (isFocused() && m_caretVisible && !readOnly()) {
        f32 caretX = pad.left + measureSubstring(m_caretPos) - m_scrollOffset;
        ctx.fillRect({caretX, pad.top, 1.5f, bounds().height - pad.verticalSum()}, caretColor());
    }

    ctx.popClip();

    // Update caret blink (simple timer, ~530ms period)
    m_caretBlinkTimer += 16.6f;  // approximate frame time
    if (m_caretBlinkTimer >= 530.0f) {
        m_caretBlinkTimer = 0.0f;
        m_caretVisible = !m_caretVisible;
    }
}

bool TextBox::onMouseEvent(const MouseEvent& event) {
    if (!isEnabled()) return false;

    switch (event.type) {
        case MouseEventType::ButtonDown:
            if (event.button == MouseButton::Left) {
                // Click-to-focus
                if (context()) {
                    context()->focusManager().setFocus(this);
                }
                i32 pos = hitTestCaret(event.position.x);
                if (event.clickCount == 2) {
                    // Double-click selects word
                    m_selAnchor = wordBoundaryLeft(pos);
                    m_caretPos = wordBoundaryRight(pos);
                } else {
                    bool shift = hasModifier(event.modifiers, ModifierKeys::Shift);
                    moveCaret(pos, shift);
                }
                // Capture mouse for drag selection
                if (context()) {
                    context()->inputManager().captureMouse(this);
                }
                return true;
            }
            break;

        case MouseEventType::ButtonUp:
            if (event.button == MouseButton::Left) {
                if (context()) {
                    context()->inputManager().releaseMouse();
                }
                return true;
            }
            break;

        case MouseEventType::Move:
            // Drag selection
            if (context() && context()->inputManager().capturedElement() == this) {
                if (context()->inputManager().isMouseButtonDown(MouseButton::Left)) {
                    i32 pos = hitTestCaret(event.position.x);
                    if (m_selAnchor < 0) m_selAnchor = m_caretPos;
                    m_caretPos = pos;
                    m_caretBlinkTimer = 0.0f;
                    m_caretVisible = true;
                    scrollToCaret();
                    invalidateRender();
                    return true;
                }
            }
            break;

        default:
            break;
    }

    return false;
}

bool TextBox::onKeyEvent(const KeyEvent& event) {
    if (!isEnabled()) return false;

    // Text input (Char events)
    if (event.isChar()) {
        if (event.character >= 32 && !event.hasControl()) {
            char buf[4];
            // Simple ASCII for now; UTF-8 extension would go here
            if (event.character < 128) {
                buf[0] = static_cast<char>(event.character);
                buf[1] = '\0';
                insertText(buf);
            }
            return true;
        }
        return false;
    }

    if (!event.isKeyDown()) return false;

    bool shift = event.hasShift();
    bool cmd = event.hasControl();  // Cmd on macOS maps to Control modifier

    switch (event.key) {
        case Key::Left:
            if (cmd) {
                moveCaret(wordBoundaryLeft(m_caretPos), shift);
            } else if (!shift && hasSelection()) {
                i32 s = selectionStart();
                clearSelection();
                moveCaret(s, false);
            } else {
                moveCaret(m_caretPos - 1, shift);
            }
            return true;

        case Key::Right:
            if (cmd) {
                moveCaret(wordBoundaryRight(m_caretPos), shift);
            } else if (!shift && hasSelection()) {
                i32 e = selectionEnd();
                clearSelection();
                moveCaret(e, false);
            } else {
                moveCaret(m_caretPos + 1, shift);
            }
            return true;

        case Key::Home:
            moveCaret(0, shift);
            return true;

        case Key::End:
            moveCaret(static_cast<i32>(text().size()), shift);
            return true;

        case Key::Backspace:
            if (cmd) {
                // Cmd+Backspace = delete to beginning of line
                if (!hasSelection()) {
                    m_selAnchor = m_caretPos;
                    m_caretPos = 0;
                }
                deleteText(-1);
            } else {
                deleteText(-1);
            }
            return true;

        case Key::Delete:
            deleteText(1);
            return true;

        case Key::Return:
        case Key::NumpadEnter:
            submitted.emit();
            if (m_onSubmit) m_onSubmit();
            return true;

        case Key::A:
            if (cmd) { selectAll(); invalidateRender(); return true; }
            break;

        case Key::C:
            if (cmd && hasSelection()) {
                // Copy — platform-specific. We'll just store in a static for now.
                // In a real app, this would go to the system clipboard.
                return true;
            }
            break;

        case Key::V:
            if (cmd) {
                // Paste — would read from system clipboard.
                return true;
            }
            break;

        case Key::X:
            if (cmd && hasSelection()) {
                // Cut — copy + delete selection
                deleteText(0);
                return true;
            }
            break;

        case Key::Tab:
            return false;  // Let tab navigate to next control

        default:
            break;
    }

    return false;
}

void TextBox::onFocusGained() {
    Element::onFocusGained();
    m_caretBlinkTimer = 0.0f;
    m_caretVisible = true;
    invalidateRender();
}

void TextBox::onFocusLost() {
    Element::onFocusLost();
    clearSelection();
    invalidateRender();
}

void TextBox::onMouseEnter() {
    Element::onMouseEnter();
    invalidateRender();
}

void TextBox::onMouseLeave() {
    Element::onMouseLeave();
    invalidateRender();
}

} // namespace gut


// --- Context.cpp ---


namespace gut {

Context::Context(std::unique_ptr<RenderBackend> backend)
    : m_backend(std::move(backend))
    , m_renderContext(std::make_unique<RenderContext>(*m_backend))
    , m_inputManager(std::make_unique<InputManager>(*this))
    , m_focusManager(std::make_unique<FocusManager>(*this))
{
}

Context::~Context() = default;

void Context::setRoot(Ref<Element> root) {
    if (m_root) {
        propagateContext(m_root.get());  // clear old
    }
    m_root = std::move(root);
    if (m_root) {
        propagateContext(m_root.get());
    }
    m_layoutDirty = true;
}

void Context::addStyleSheet(Ref<StyleSheet> stylesheet) {
    m_styleSheets.push_back(std::move(stylesheet));
}

void Context::setTheme(Ref<Theme> theme) {
    m_theme = std::move(theme);
}

void Context::update(f32 deltaTime) {
    m_totalTime += static_cast<f64>(deltaTime);
    
    // Update global animation timeline
    Timeline::global()->update(deltaTime);
    
    // Animations may have changed layout properties (e.g., width),
    // so mark layout dirty to re-measure on next render
    m_layoutDirty = true;
}

void Context::layout() {
    if (!m_root || !m_layoutDirty) return;
    
    // Apply styles first
    applyStyles();
    
    // Two-pass layout: measure then arrange
    m_root->measure(m_size);
    m_root->arrange(Rectf{0, 0, m_size.width, m_size.height});
    
    m_layoutDirty = false;
}

void Context::render(f32 width, f32 height, f32 devicePixelRatio) {
    // Update size if changed
    if (m_size.width != width || m_size.height != height) {
        m_size = {width, height};
        m_layoutDirty = true;
    }
    
    // Ensure layout is up to date
    layout();
    
    if (!m_root) return;
    
    // Begin frame on render context (which will call backend->beginFrame)
    m_renderContext->beginFrame({width, height}, devicePixelRatio);
    
    // Render the element tree
    renderElement(m_root.get());

    // Render overlays (popups, tooltips) on top of everything
    for (auto& [owner, fn] : m_overlays) {
        fn(*m_renderContext);
    }

    // End frame on render context (which will flush and call backend->endFrame)
    m_renderContext->endFrame();
}

void Context::renderElement(Element* element) {
    if (!element) return;
    if (element->visibility() != Visibility::Visible) return;
    
    // Render this element (Panel::onRender handles children via renderChildren,
    // so we do NOT iterate children here — that would double-render them
    // outside the parent's opacity/clip scope)
    element->render(*m_renderContext);
}

void Context::processMouseMove(f32 x, f32 y) {
    m_inputManager->processMouseMove(x, y);
}

void Context::processMouseButton(MouseButton button, bool pressed) {
    m_inputManager->processMouseButton(button, pressed);
}

void Context::processMouseWheel(f32 deltaX, f32 deltaY) {
    m_inputManager->processMouseWheel(deltaX, deltaY);
}

void Context::processKey(Key key, bool pressed, ModifierKeys modifiers) {
    m_inputManager->processKey(key, pressed, modifiers);
}

void Context::processTextInput(char32_t character) {
    m_inputManager->processTextInput(character);
}

Element* Context::findById(StringView id) {
    if (!m_root) return nullptr;
    return findElementById(m_root.get(), id);
}

Element* Context::findElementById(Element* element, StringView id) {
    if (element->id() == id) return element;
    
    for (usize i = 0; i < element->childCount(); ++i) {
        if (auto* found = findElementById(element->childAt(i), id)) {
            return found;
        }
    }
    return nullptr;
}

std::vector<Element*> Context::findByClass(StringView className) {
    std::vector<Element*> results;
    if (m_root) {
        findElementsByClass(m_root.get(), className, results);
    }
    return results;
}

void Context::findElementsByClass(Element* element, StringView className, 
                                   std::vector<Element*>& results) {
    if (element->className() == className) {
        results.push_back(element);
    }
    for (usize i = 0; i < element->childCount(); ++i) {
        findElementsByClass(element->childAt(i), className, results);
    }
}

Element* Context::hitTest(Point2f point) {
    if (!m_root) return nullptr;
    return hitTestElement(m_root.get(), point);
}

Element* Context::hitTestElement(Element* element, Point2f point) {
    if (element->visibility() != Visibility::Visible) return nullptr;
    if (!element->isEnabled()) return nullptr;
    
    // Check children first (front to back, last child is on top)
    for (usize i = element->childCount(); i > 0; --i) {
        if (auto* hit = hitTestElement(element->childAt(i - 1), point)) {
            return hit;
        }
    }
    
    // Check this element
    if (element->hitTest(point)) {
        return element;
    }
    
    return nullptr;
}

void Context::setSize(f32 width, f32 height) {
    if (m_size.width != width || m_size.height != height) {
        m_size = {width, height};
        m_layoutDirty = true;
    }
}

void Context::addOverlay(Element* owner, std::function<void(RenderContext&)> renderFn) {
    // Replace if already registered
    for (auto& [o, fn] : m_overlays) {
        if (o == owner) {
            fn = std::move(renderFn);
            return;
        }
    }
    m_overlays.emplace_back(owner, std::move(renderFn));
}

void Context::removeOverlay(Element* owner) {
    m_overlays.erase(
        std::remove_if(m_overlays.begin(), m_overlays.end(),
                        [owner](const auto& p) { return p.first == owner; }),
        m_overlays.end());
}

void Context::propagateContext(Element* element) {
    if (!element) return;
    element->m_context = this;
    for (usize i = 0; i < element->childCount(); ++i) {
        propagateContext(element->childAt(i));
    }
}

void Element::setContext(Context* ctx) {
    m_context = ctx;
    // Propagate to children for Panel-derived elements
    for (usize i = 0; i < childCount(); ++i) {
        childAt(i)->setContext(ctx);
    }
}

void Context::applyStyles() {
    if (!m_root) return;
    applyStyleToElement(m_root.get());
}

void Context::applyStyleToElement(Element* element) {
    if (!element) return;
    
    // Collect style properties from all stylesheets + theme
    std::unordered_map<String, StyleValue> merged;
    
    // Theme styles (lower priority)
    if (m_theme) {
        auto style = m_theme->computeStyle(*element);
        for (const auto& [name, value] : style->properties()) {
            merged[name] = value;
        }
    }
    
    // Context-level stylesheets (higher priority)
    for (const auto& sheet : m_styleSheets) {
        auto rules = sheet->matchingRules(*element);
        for (const auto* rule : rules) {
            for (const auto& [name, value] : rule->properties()) {
                merged[name] = value;
            }
        }
    }
    
    // Apply merged properties to element
    for (const auto& [name, value] : merged) {
        if (name == "opacity" && std::holds_alternative<f32>(value)) {
            element->setopacity(std::get<f32>(value));
        } else if (name == "visibility" && std::holds_alternative<Visibility>(value)) {
            element->setvisibility(std::get<Visibility>(value));
        } else if (name == "margin" && std::holds_alternative<Thickness>(value)) {
            element->setmargin(std::get<Thickness>(value));
        } else if (name == "padding" && std::holds_alternative<Thickness>(value)) {
            element->setpadding(std::get<Thickness>(value));
        } else if (name == "width" && std::holds_alternative<f32>(value)) {
            element->setwidth(std::get<f32>(value));
        } else if (name == "height" && std::holds_alternative<f32>(value)) {
            element->setheight(std::get<f32>(value));
        } else if (name == "min-width" && std::holds_alternative<f32>(value)) {
            element->setminWidth(std::get<f32>(value));
        } else if (name == "min-height" && std::holds_alternative<f32>(value)) {
            element->setminHeight(std::get<f32>(value));
        } else if (name == "max-width" && std::holds_alternative<f32>(value)) {
            element->setmaxWidth(std::get<f32>(value));
        } else if (name == "max-height" && std::holds_alternative<f32>(value)) {
            element->setmaxHeight(std::get<f32>(value));
        } else if (name == "horizontal-alignment" && std::holds_alternative<HorizontalAlignment>(value)) {
            element->sethorizontalAlignment(std::get<HorizontalAlignment>(value));
        } else if (name == "vertical-alignment" && std::holds_alternative<VerticalAlignment>(value)) {
            element->setverticalAlignment(std::get<VerticalAlignment>(value));
        } else if (name == "background" && std::holds_alternative<Color>(value)) {
            // background is on Panel and Button, not Element
            if (auto* panel = dynamic_cast<Panel*>(element)) {
                panel->setbackground(std::get<Color>(value));
            } else if (auto* btn = dynamic_cast<Button*>(element)) {
                btn->setbackground(std::get<Color>(value));
            }
        } else if (name == "foreground" && std::holds_alternative<Color>(value)) {
            if (auto* text = dynamic_cast<Text*>(element)) {
                text->setforeground(std::get<Color>(value));
            } else if (auto* btn = dynamic_cast<Button*>(element)) {
                btn->setforeground(std::get<Color>(value));
            }
        } else if (name == "font-size" && std::holds_alternative<f32>(value)) {
            if (auto* text = dynamic_cast<Text*>(element)) {
                text->setfontSize(std::get<f32>(value));
            }
        }
    }
    
    // Recurse into children
    for (usize i = 0; i < element->childCount(); ++i) {
        applyStyleToElement(element->childAt(i));
    }
}

Ref<Font> Context::loadFont(const u8* data, size_t size) {
    auto font = makeRef<Font>();
    if (!font->loadFromMemory(data, size)) {
        return nullptr;
    }
    
    // Auto-register in the font registry
    registerFont(font);
    
    // Set as default font if we don't have one yet
    if (!m_defaultFont) {
        m_defaultFont = font;
    }
    
    return font;
}

Ref<Font> Context::loadFont(const u8* data, size_t size,
                            const std::string& family,
                            FontWeight weight,
                            FontStyle  style) {
    auto font = makeRef<Font>();
    // Pre-set metadata before loading so initStbFont() won't overwrite
    font->setFamily(family);
    font->setWeight(weight);
    font->setStyle(style);
    if (!font->loadFromMemory(data, size)) {
        return nullptr;
    }
    
    registerFont(font);
    
    if (!m_defaultFont) {
        m_defaultFont = font;
    }
    
    return font;
}

void Context::registerFont(Ref<Font> font) {
    if (!font) return;
    FontKey key{font->family(), font->weight(), font->style()};
    m_fontRegistry[key] = font;
}

Font* Context::findFont(const std::string& family,
                        FontWeight weight,
                        FontStyle  style) const {
    if (m_fontRegistry.empty())
        return m_defaultFont.get();
    
    // Resolve generic family names ("sans-serif", "", etc.) to the default
    // font's actual family so that weight/style selection works.
    const std::string& resolvedFamily =
        (!family.empty() && family != "sans-serif" && family != "serif"
         && family != "monospace")
            ? family
            : (m_defaultFont ? m_defaultFont->family() : family);
    
    // 1. Exact match
    {
        auto it = m_fontRegistry.find(FontKey{resolvedFamily, weight, style});
        if (it != m_fontRegistry.end())
            return it->second.get();
    }
    
    // 2. Same family + style, nearest weight
    {
        Font* best = nullptr;
        int bestDist = INT_MAX;
        for (auto& [key, f] : m_fontRegistry) {
            if (key.family == resolvedFamily && key.style == style) {
                int dist = std::abs(static_cast<int>(key.weight)
                                  - static_cast<int>(weight));
                if (dist < bestDist) {
                    bestDist = dist;
                    best = f.get();
                }
            }
        }
        if (best) return best;
    }
    
    // 3. Same family, any weight/style
    {
        for (auto& [key, f] : m_fontRegistry) {
            if (key.family == resolvedFamily)
                return f.get();
        }
    }
    
    // 4. Fallback to default
    return m_defaultFont.get();
}

void Context::setDefaultFont(Ref<Font> font) {
    m_defaultFont = std::move(font);
}

} // namespace gut


// --- Gut.cpp ---


namespace gut {

namespace {
    constexpr i32 VERSION_MAJOR = 1;
    constexpr i32 VERSION_MINOR = 0;
    constexpr i32 VERSION_PATCH = 0;
    constexpr const char* VERSION_STRING = "1.0.0";
}

i32 versionMajor() {
    return VERSION_MAJOR;
}

i32 versionMinor() {
    return VERSION_MINOR;
}

i32 versionPatch() {
    return VERSION_PATCH;
}

const char* versionString() {
    return VERSION_STRING;
}

bool initialize() {
    // Initialize subsystems
    // - Timeline is lazily initialized
    // - ResourceManager is lazily initialized
    // - Input managers are per-context
    return true;
}

void shutdown() {
    // Cleanup global instances
    // Note: In a real implementation, we'd have proper cleanup of:
    // - Global timeline
    // - Global resource manager
    // - Any other global state
}

} // namespace gut


#endif // GUT_IMPLEMENTATION

