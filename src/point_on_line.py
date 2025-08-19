#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
计算点在直线上的参数值
"""

import math

class Point3D:
    """3D点类"""
    def __init__(self, x=0, y=0, z=0):
        self.x = float(x)
        self.y = float(y)
        self.z = float(z)
    
    def __sub__(self, other):
        """向量减法"""
        return Point3D(self.x - other.x, self.y - other.y, self.z - other.z)
    
    def __add__(self, other):
        """向量加法"""
        return Point3D(self.x + other.x, self.y + other.y, self.z + other.z)
    
    def __mul__(self, scalar):
        """标量乘法"""
        return Point3D(self.x * scalar, self.y * scalar, self.z * scalar)
    
    def dot(self, other):
        """点积"""
        return self.x * other.x + self.y * other.y + self.z * other.z
    
    def length(self):
        """向量长度"""
        return math.sqrt(self.x**2 + self.y**2 + self.z**2)
    
    def normalize(self):
        """归一化"""
        length = self.length()
        if length == 0:
            return Point3D()
        return Point3D(self.x / length, self.y / length, self.z / length)
    
    def __str__(self):
        return f"({self.x:.6f}, {self.y:.6f}, {self.z:.6f})"

def get_parameter_on_line(line_point, line_direction, test_point):
    """
    计算点在直线上的参数值
    
    参数:
        line_point: 直线上的一点
        line_direction: 直线的方向向量
        test_point: 要测试的点
    
    返回:
        参数值t，使得 test_point = line_point + t * normalized_direction
    """
    normalized_dir = line_direction.normalize()
    v = test_point - line_point
    return v.dot(normalized_dir)

def distance_to_line(line_point, line_direction, test_point):
    """
    计算点到直线的距离
    
    参数:
        line_point: 直线上的一点
        line_direction: 直线的方向向量
        test_point: 要测试的点
    
    返回:
        点到直线的距离
    """
    normalized_dir = line_direction.normalize()
    v = test_point - line_point
    t = v.dot(normalized_dir)
    closest_point = line_point + normalized_dir * t
    return (test_point - closest_point).length()

def is_point_on_line(line_point, line_direction, test_point, tolerance=1e-10):
    """
    判断点是否在直线上
    
    参数:
        line_point: 直线上的一点
        line_direction: 直线的方向向量
        test_point: 要测试的点
        tolerance: 容差
    
    返回:
        True如果点在直线上，False否则
    """
    return distance_to_line(line_point, line_direction, test_point) < tolerance

def main():
    print("=== Python版本：计算点在直线上的参数值 ===")
    
    # 定义直线：通过点(0,0,0)和方向(1,1,1)
    line_point = Point3D(0, 0, 0)
    line_direction = Point3D(1, 1, 1)
    
    print(f"直线定义：")
    print(f"  点: {line_point}")
    print(f"  方向: {line_direction}")
    
    # 测试点
    test_points = [
        Point3D(2, 2, 2),      # 在直线上
        Point3D(0.5, 0.5, 0.5), # 在直线上
        Point3D(1, 0, 0),      # 不在直线上
        Point3D(-1, -1, -1),   # 在直线反向延长线上
        Point3D(3, 3, 3),      # 在直线延长线上
        Point3D(0, 0, 0)       # 在直线上（参考点）
    ]
    
    print(f"\n测试结果：")
    for i, point in enumerate(test_points, 1):
        t = get_parameter_on_line(line_point, line_direction, point)
        dist = distance_to_line(line_point, line_direction, point)
        on_line = is_point_on_line(line_point, line_direction, point)
        
        print(f"点{i} {point}:")
        print(f"  参数值 t = {t:.6f}")
        print(f"  到直线距离 = {dist:.6f}")
        
        if on_line:
            print(f"  → 点在直线上")
            if abs(t) < 1e-10:
                print(f"  → 点正好在参考点处 (t ≈ 0)")
            elif t > 0:
                print(f"  → 点在直线正方向上")
            else:
                print(f"  → 点在直线反方向上")
        else:
            print(f"  → 点不在直线上")
        print()
    
    # 验证：根据参数值计算点
    print("验证：根据参数值计算点")
    test_t = 2.5
    normalized_dir = line_direction.normalize()
    computed_point = line_point + normalized_dir * test_t
    print(f"参数值 t = {test_t} 对应的点: {computed_point}")
    
    # 验证计算是否正确
    actual_t = get_parameter_on_line(line_point, line_direction, computed_point)
    print(f"验证：该点的参数值 t = {actual_t:.6f}")

if __name__ == "__main__":
    main()