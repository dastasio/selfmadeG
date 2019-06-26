def ExtremesToCollider(MinX, MinY, MinZ, MaxX, MaxY, MaxZ):
    CenterX = (MinX + MaxX) / 2.0
    CenterY = (MinY + MaxY) / 2.0
    CenterZ = (MinZ + MaxZ) / 2.0
    RadiusX = (MaxX - MinX) / 2.0 #(abs(CenterX - MinX) + abs(CenterX - MaxX)) / 2.0
    RadiusY = (MaxY - MinY) / 2.0 #(abs(CenterY - MinY) + abs(CenterY - MaxY)) / 2.0
    RadiusZ = (MaxZ - MinZ) / 2.0 #(abs(CenterZ - MinZ) + abs(CenterZ - MaxZ)) / 2.0

    print("Center = {{{0:.6f}f, {1:.6f}f, {2:.6f}f}};".format(CenterX, CenterY, CenterZ))
    print("Radius = {{{0:.6f}f, {1:.6f}f, {2:.6f}f}};".format(RadiusX, RadiusY, RadiusZ))

if __name__ == "__main__":
    MinX = float(input("MinX: ").replace('m', ''))
    MinY = float(input("MinY: ").replace('m', ''))
    MinZ = float(input("MinZ: ").replace('m', ''))

    MaxX = float(input("MaxX: ").replace('m', ''))
    MaxY = float(input("MaxY: ").replace('m', ''))
    MaxZ = float(input("MaxZ: ").replace('m', ''))

    ExtremesToCollider(MinX, MinZ, -MaxY, MaxX, MaxZ, -MinY)