#nullable enable

namespace Panda3D.Core;

// Reverse-operand scalar multiplication (scalar * v), mirroring Python __rmul__.
// C# operators are not inherited, so each concrete type that has a forward
// operator *(T, scalar) gets its commutative partner here. Delegates to the
// forward operator.
public partial class LMatrix3d { public static LMatrix3d operator *(double scalar, LMatrix3d v) => v * scalar; }
public partial class LMatrix3f { public static LMatrix3f operator *(float scalar, LMatrix3f v) => v * scalar; }
public partial class LMatrix4d { public static LMatrix4d operator *(double scalar, LMatrix4d v) => v * scalar; }
public partial class LMatrix4f { public static LMatrix4f operator *(float scalar, LMatrix4f v) => v * scalar; }
public partial class LPoint2d { public static LPoint2d operator *(double scalar, LPoint2d v) => v * scalar; }
public partial class LPoint2f { public static LPoint2f operator *(float scalar, LPoint2f v) => v * scalar; }
public partial class LPoint2i { public static LPoint2i operator *(int scalar, LPoint2i v) => v * scalar; }
public partial class LPoint3d { public static LPoint3d operator *(double scalar, LPoint3d v) => v * scalar; }
public partial class LPoint3f { public static LPoint3f operator *(float scalar, LPoint3f v) => v * scalar; }
public partial class LPoint3i { public static LPoint3i operator *(int scalar, LPoint3i v) => v * scalar; }
public partial class LPoint4d { public static LPoint4d operator *(double scalar, LPoint4d v) => v * scalar; }
public partial class LPoint4f { public static LPoint4f operator *(float scalar, LPoint4f v) => v * scalar; }
public partial class LPoint4i { public static LPoint4i operator *(int scalar, LPoint4i v) => v * scalar; }
public partial class LQuaterniond { public static LQuaterniond operator *(double scalar, LQuaterniond v) => v * scalar; }
public partial class LQuaternionf { public static LQuaternionf operator *(float scalar, LQuaternionf v) => v * scalar; }
public partial class LRotationd { public static LRotationd operator *(double scalar, LRotationd v) => v * scalar; }
public partial class LRotationf { public static LRotationf operator *(float scalar, LRotationf v) => v * scalar; }
public partial class LVecBase2d { public static LVecBase2d operator *(double scalar, LVecBase2d v) => v * scalar; }
public partial class LVecBase2f { public static LVecBase2f operator *(float scalar, LVecBase2f v) => v * scalar; }
public partial class LVecBase2i { public static LVecBase2i operator *(int scalar, LVecBase2i v) => v * scalar; }
public partial class LVecBase3d { public static LVecBase3d operator *(double scalar, LVecBase3d v) => v * scalar; }
public partial class LVecBase3f { public static LVecBase3f operator *(float scalar, LVecBase3f v) => v * scalar; }
public partial class LVecBase3i { public static LVecBase3i operator *(int scalar, LVecBase3i v) => v * scalar; }
public partial class LVecBase4d { public static LVecBase4d operator *(double scalar, LVecBase4d v) => v * scalar; }
public partial class LVecBase4f { public static LVecBase4f operator *(float scalar, LVecBase4f v) => v * scalar; }
public partial class LVecBase4i { public static LVecBase4i operator *(int scalar, LVecBase4i v) => v * scalar; }
public partial class LVector2d { public static LVector2d operator *(double scalar, LVector2d v) => v * scalar; }
public partial class LVector2f { public static LVector2f operator *(float scalar, LVector2f v) => v * scalar; }
public partial class LVector2i { public static LVector2i operator *(int scalar, LVector2i v) => v * scalar; }
public partial class LVector3d { public static LVector3d operator *(double scalar, LVector3d v) => v * scalar; }
public partial class LVector3f { public static LVector3f operator *(float scalar, LVector3f v) => v * scalar; }
public partial class LVector3i { public static LVector3i operator *(int scalar, LVector3i v) => v * scalar; }
public partial class LVector4d { public static LVector4d operator *(double scalar, LVector4d v) => v * scalar; }
public partial class LVector4f { public static LVector4f operator *(float scalar, LVector4f v) => v * scalar; }
public partial class LVector4i { public static LVector4i operator *(int scalar, LVector4i v) => v * scalar; }
