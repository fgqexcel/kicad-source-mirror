/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2018 CERN
 * Copyright (C) 2019-2020 KiCad Developers, see AUTHORS.txt for contributors.
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef __SHAPE_ARC_H
#define __SHAPE_ARC_H

#include <geometry/seg.h>
#include <geometry/shape.h>
#include <math/box2.h>       // for BOX2I
#include <math/vector2d.h>   // for VECTOR2I

class SHAPE_LINE_CHAIN;

class SHAPE_ARC : public SHAPE
{
public:
    SHAPE_ARC() :
        SHAPE( SH_ARC ), m_centralAngle( 0.0 ), m_width( 0 ) {};

    /**
     * SHAPE_ARC ctor.
     * @param aArcCenter is the arc center
     * @param aArcStartPoint is the arc start point
     * @param aCenterAngle is the arc angle in degrees
     * @param aWidth is the arc line thickness
     */
    SHAPE_ARC( const VECTOR2I& aArcCenter, const VECTOR2I& aArcStartPoint,
               double aCenterAngle, int aWidth = 0 ) :
        SHAPE( SH_ARC ), m_p0( aArcStartPoint ), m_pc( aArcCenter ), m_centralAngle( aCenterAngle ),
        m_width( aWidth )
    {
        update_bbox();
    }

    SHAPE_ARC( const SHAPE_ARC& aOther )
        : SHAPE( SH_ARC )
    {
        m_p0 = aOther.m_p0;
        m_pc = aOther.m_pc;
        m_centralAngle = aOther.m_centralAngle;
        m_width = aOther.m_width;
        m_bbox = aOther.m_bbox;
    }

    ~SHAPE_ARC() {}

    SHAPE* Clone() const override
    {
        return new SHAPE_ARC( *this );
    }

    const VECTOR2I& GetP0() const { return m_p0; }
    const VECTOR2I GetP1() const;
    const VECTOR2I GetArcMid() const;
    const VECTOR2I& GetCenter() const { return m_pc; }

    const BOX2I BBox( int aClearance = 0 ) const override;

    bool Collide( const SEG& aSeg, int aClearance = 0 ) const override;
    bool Collide( const VECTOR2I& aP, int aClearance = 0 ) const override;

    void SetWidth( int aWidth )
    {
        m_width = aWidth;
    }

    int GetWidth() const
    {
        return m_width;
    }

    bool IsSolid() const override
    {
        return true;
    }

    void Move( const VECTOR2I& aVector ) override
    {
        m_p0 += aVector;
        m_pc += aVector;
        update_bbox();
    }

    /**
     * Function Rotate
     * rotates the arc by a given angle about a point
     * @param aCenter is the rotation center
     * @param aAngle rotation angle in radians
     */
    void Rotate( double aAngle, const VECTOR2I& aCenter )
    {
        m_p0 -= aCenter;
        m_pc -= aCenter;

        m_p0.Rotate( aAngle );
        m_pc.Rotate( aAngle );

        m_pc += aCenter;
        m_p0 += aCenter;
        update_bbox();
    }

    void Mirror( bool aX = true, bool aY = false, const VECTOR2I& aVector = { 0, 0 } )
    {
        if( aX )
        {
            m_p0.x = -m_p0.x + 2 * aVector.x;
            m_pc.x = -m_pc.x + 2 * aVector.x;
            m_centralAngle = - m_centralAngle;
        }

        if( aY )
        {
            m_p0.y = -m_p0.y + 2 * aVector.y;
            m_pc.y = -m_pc.y + 2 * aVector.y;
            m_centralAngle = - m_centralAngle;
        }

        update_bbox();
    }

    int GetRadius() const;

    SEG GetChord() const
    {
        return SEG( m_p0, GetP1() );
    }

    double  GetCentralAngle() const;
    double  GetStartAngle() const;
    double  GetEndAngle() const;

/*
    bool ConstructFromCorners( VECTOR2I aP0, VECTOR2I aP1, double aCenterAngle );
    bool ConstructFromCircle( VECTOR2I aP0, double aRadius );

    bool ConstructFromCenterAndAngles( VECTOR2I aCenter, double aRadius, double aStartAngle, double aCenterAngle );

    bool ConstructFromCornerAndAngles( VECTOR2I aP0,
            double aStartAngle,
            double aCenterAngle,
            double aRadius );
*/

    /**
     * Constructs a SHAPE_LINE_CHAIN of segments from a given arc
     * @param aAccuracy maximum divergence from true arc given in internal units
     *   ** Note that the default of 500.0 here is given using ARC_DEF_HIGH_ACCURACY
     *      for pcbnew units.  This is to allow common geometry collision functions
     *      Other programs should call this using explicit accuracy values
     *      TODO: unify KiCad internal units
     *
     * @return a SHAPE_LINE_CHAIN
     */
    const SHAPE_LINE_CHAIN ConvertToPolyline( double aAccuracy = 500.0 ) const;

private:

    bool ccw( const VECTOR2I& aA, const VECTOR2I& aB, const VECTOR2I& aC ) const
    {
        return (ecoord) ( aC.y - aA.y ) * ( aB.x - aA.x ) >
               (ecoord) ( aB.y - aA.y ) * ( aC.x - aA.x );
    }

    void update_bbox();


    VECTOR2I m_p0, m_pc;
    double m_centralAngle;

    int m_width;
    BOX2I m_bbox;
};

#endif
