#nullable enable

namespace Panda3D.Core;

public partial class LVecBase2f {
  public float this[int index] {
    get => GetCell(index);
    set => SetCell(index, value);
  }

  public void Deconstruct(out float x, out float y) {
    x = X;
    y = Y;
  }
}

public partial class LVecBase2d {
  public double this[int index] {
    get => GetCell(index);
    set => SetCell(index, value);
  }

  public void Deconstruct(out double x, out double y) {
    x = X;
    y = Y;
  }
}

public partial class LVecBase2i {
  public int this[int index] {
    get => GetCell(index);
    set => SetCell(index, value);
  }

  public void Deconstruct(out int x, out int y) {
    x = X;
    y = Y;
  }
}

public partial class LVecBase3f {
  public float this[int index] {
    get => GetCell(index);
    set => SetCell(index, value);
  }

  public void Deconstruct(out float x, out float y, out float z) {
    x = X;
    y = Y;
    z = Z;
  }
}

public partial class LVecBase3d {
  public double this[int index] {
    get => GetCell(index);
    set => SetCell(index, value);
  }

  public void Deconstruct(out double x, out double y, out double z) {
    x = X;
    y = Y;
    z = Z;
  }
}

public partial class LVecBase3i {
  public int this[int index] {
    get => GetCell(index);
    set => SetCell(index, value);
  }

  public void Deconstruct(out int x, out int y, out int z) {
    x = X;
    y = Y;
    z = Z;
  }
}

public partial class LVecBase4f {
  public float this[int index] {
    get => GetCell(index);
    set => SetCell(index, value);
  }

  public void Deconstruct(out float x, out float y, out float z, out float w) {
    x = X;
    y = Y;
    z = Z;
    w = GetCell(3);
  }
}

public partial class LVecBase4d {
  public double this[int index] {
    get => GetCell(index);
    set => SetCell(index, value);
  }

  public void Deconstruct(out double x, out double y, out double z, out double w) {
    x = X;
    y = Y;
    z = Z;
    w = GetCell(3);
  }
}

public partial class LVecBase4i {
  public int this[int index] {
    get => GetCell(index);
    set => SetCell(index, value);
  }

  public void Deconstruct(out int x, out int y, out int z, out int w) {
    x = X;
    y = Y;
    z = Z;
    w = GetCell(3);
  }
}

public partial class LMatrix3f {
  public float this[int row, int column] {
    get => GetCell(row, column);
    set => SetCell(row, column, value);
  }

  public void Deconstruct(out LVecBase3f row0, out LVecBase3f row1, out LVecBase3f row2) {
    row0 = GetRow(0);
    row1 = GetRow(1);
    row2 = GetRow(2);
  }
}

public partial class LMatrix3d {
  public double this[int row, int column] {
    get => GetCell(row, column);
    set => SetCell(row, column, value);
  }

  public void Deconstruct(out LVecBase3d row0, out LVecBase3d row1, out LVecBase3d row2) {
    row0 = GetRow(0);
    row1 = GetRow(1);
    row2 = GetRow(2);
  }
}

public partial class LMatrix4f {
  public float this[int row, int column] {
    get => GetCell(row, column);
    set => SetCell(row, column, value);
  }

  public void Deconstruct(out LVecBase4f row0, out LVecBase4f row1, out LVecBase4f row2, out LVecBase4f row3) {
    row0 = GetRow(0);
    row1 = GetRow(1);
    row2 = GetRow(2);
    row3 = GetRow(3);
  }
}

public partial class LMatrix4d {
  public double this[int row, int column] {
    get => GetCell(row, column);
    set => SetCell(row, column, value);
  }

  public void Deconstruct(out LVecBase4d row0, out LVecBase4d row1, out LVecBase4d row2, out LVecBase4d row3) {
    row0 = GetRow(0);
    row1 = GetRow(1);
    row2 = GetRow(2);
    row3 = GetRow(3);
  }
}
