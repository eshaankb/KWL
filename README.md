
# **KWL Language Specification (v1.1)**

**Kompact Workhorse Language** | **Extension:** `.kwl` | **Target:** Native Machine Code

## **1. Lexical & Syntax Essentials**

* **Grouping:** `\` replaces `(` and `)`.
* *Note*: `\` is distinct from `\\`
* **Blocks:** `[` and `]` replace `{` and `}`.
* **Member Access/Namespace:** `,` replaces `.` (e.g., `math,pi`).
* **Line End:** `;` is required.
* **No-Shift Design:** Uses `\` and `,` heavily to keep fingers on the home row.

---

## **2. Low-Level Control & Memory**

To provide "C-level" power without the complexity of C++, KWL introduces **Direct Addressing**.

### **2.1 Pointers and Addresses**

* **`adr\var\`**: Returns the memory address of a variable (Pointer).
* **`val\ptr\`**: Dereferences a pointer to get the value.
* **`off\ptr; bytes\`**: Offsets a pointer by a specific byte count.

### **2.2 Manual Memory**

* **`heap\size\`**: Allocates a raw block of memory on the heap.
* **`dest\target\`**: Immediately frees memory and nulls the identifier.
* **`stack\var\`**: Forces a variable to stay on the stack (useful for high-speed local math).

---

## **3. Control Flow & Loops**

### **3.1 The lp (Loop) Keyword**

* **Infinite Loop:** `lp [ ... ]`
* **Conditional Loop (While):** `lp \x < 10\ [ ... ]`
* **Iterative Loop (For):** `lp \int i = 0; i < 10; i++\ [ ... ]`
* **Collection Loop (Each):** `lp \item : my_array\ [ ... ]`

### **3.2 Branching**

* `if \cond\ [ ] el \cond\ [ ] el [ ]`
* **`brk;`**: Breaks out of a loop.
* **`con;`**: Continues to the next iteration.

---

## **4. Modules: Import & Export**

KWL uses a **Selective Exposure** model. The compiler only looks at what you explicitly provide.

### **4.1 Exporting (Defining a Module)**

To make a file a module, use the `pub` (public) keyword.

```kwl
# file: math_tools.kwl
pub int64`fn double_val\int64 n\ [
    return \n * 2\;
]

pub fl64 gbl piconst = 3.14159;

```

### **4.2 Importing**

* **`import\path\with name;`**: Standard import.
* **`import\path\all;`**: Import everything into the global namespace.

```kwl
import\standard\>loginout\ with log;
import\math_tools\ with mt;

void`fn main\\ [
    log,wt\mt,double_val\21\\;
]

```

---

## **5. System Architecture (ECS & Game Runtime)**

### **5.1 The Data-Oriented Pipeline**

KWL treats classes (`crclass`) as data containers.

* **`crclass`**: Data only (attributes).
* **`tobj`**: Allows methods.
* **`constr`**: A `void` function used as a constructor.

```kwl
crclass transform [
    vec2\fl\ pos;
    fl rot;
    void`fn constr\fl rot; vec2\fl\ pos\;
]

# This system runs every frame automatically on its own Fiber
sys`update_physics [
    lp \transform,t\ [ 
        t,pos,x += 1.0; 
    ]
]

```

---

## **6. Type System Reference**

| Category | Type | Description |
| --- | --- | --- |
| **Numeric** | `int`, `int64`, `fl`, `fl64` | Signed integers and floats (32/64 bit). |
| **Text** | `str` | UTF-8 String (Heap-allocated). |
| **Logic** | `bool` | Boolean (`true`/`false`). |
| **Collections** | `arr\T\`, `mtx\T\` | Dynamic arrays and multi-dim matrices. Elements are delimited by  |
| **Geometry** | `vec2\T\`, `vec3\T\` | Native SIMD-optimized vectors. |
| **Graphics** | `sprite`, `tex` | Managed graphics handles for `glab`. |

---

## **7. KWL Standard Library (KSL)**

### **7.1 KWL,core**

* **`type\var\`**: Returns string representation of a type.
* **`adr\var\`** / **`val\ptr\`**: Memory manipulation.

### **7.2 loginout**

* **`wt\'text' or \var\`**: Writes to standard output.
* **`nline;`**: Newline.
* **`fmstr\'... |var| ...'\`**: Interpolated string (uses pipes).

### **7.3 glab (Graphics Laboratory)**

* **`init\win_name; w; h\`**: Opens a native window.
* **`clr\vec3\clr\\`**: Clears screen.
* **`flip\\`**: Buffer swap.

### **7.4 OSr (Operating System Resources)**

* **`get_os\\`**: Returns 'WIN', 'MAC', or 'LIN'. (Note: Not compatible with Chrome/Web).
* **`fs_read\path\`**: High-speed file access.

---

## **8. Example: Full Implementation**

### **player.kwl**

```kwl
import\standard\>glab\ with gl;

pub crclass Player [
    pub vec2\fl\ pos;
    pub fl speed;
    str name; # Private to module
]

pub void`fn move\Player p; vec2\fl\ dir\ [
    p,pos,x += \dir,x * p,speed\;
    p,pos,y += \dir,y * p,speed\;
]

```

### **main.kwl**

```kwl
import\standard\>loginout\ with log;
import\'player.kwl'\>player\ with ply;

void`fn main\\ [
    # Static allocation
    ply,Player hero = [ [0.0, 0.0], 5.0 ];

    lp \true\ [
        ply,move\hero; [1.0, 0.0]\;
        log,wt\log,fmstr'Hero is at |hero,pos,x|'\;

        # Memory boundary check
        if \adr\hero\ > 0xFFFFFFFF\ [
            log,wt\'Memory overflow warning'\;
            brk;
        ]
    ]

    dest\hero\; 
]
main\;

```

---
