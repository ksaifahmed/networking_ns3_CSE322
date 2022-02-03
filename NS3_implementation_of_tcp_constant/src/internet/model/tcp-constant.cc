/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 NITK Surathkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Apoorva Bhargava <apoorvabhargava13@gmail.com>
 *         Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 *
 */

#include "tcp-constant.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpConstant");
NS_OBJECT_ENSURE_REGISTERED (TcpConstant);

TypeId
TcpConstant::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpConstant")
    .SetParent<TcpCongestionOps> ()
    .SetGroupName ("Internet")
    .AddConstructor<TcpConstant> ()
    .AddTraceSource("EstimatedBW", "The estimated bandwidth",
            MakeTraceSourceAccessor(&TcpConstant::m_currentBW),
            "ns3::TracedValueCallback::Double")
  ;
  return tid;
}

TcpConstant::TcpConstant (void) :
  TcpCongestionOps (),
  m_currentBW (0),
  m_ackedSegments (0)
{
  NS_LOG_FUNCTION (this);
}

TcpConstant::TcpConstant (const TcpConstant& sock):
  TcpCongestionOps (sock),
  m_currentBW(sock.m_currentBW),
  m_ackedSegments(sock.m_ackedSegments)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("Invoked the copy constructor");
}

TcpConstant::~TcpConstant (void)
{
}

uint32_t
TcpConstant::SlowStart (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (segmentsAcked >= 1)
    {
      uint32_t sndCwnd = tcb->m_cWnd;
      tcb->m_cWnd = std::min ((sndCwnd + (segmentsAcked * tcb->m_segmentSize)), (uint32_t)tcb->m_ssThresh);
      NS_LOG_INFO ("In SlowStart, updated to cwnd " << tcb->m_cWnd << " ssthresh " << tcb->m_ssThresh);
      return segmentsAcked - ((tcb->m_cWnd - sndCwnd) / tcb->m_segmentSize);
    }

  return 0;
}

void
TcpConstant::EstimateBW (const Time &rtt, Ptr<TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (!rtt.IsZero ());

  m_currentBW = m_ackedSegments * tcb->m_segmentSize / rtt.GetSeconds ();


  // Time currentAck = Simulator::Now ();
  // m_currentBW = m_ackedSegments * tcb->m_segmentSize / (currentAck - m_lastAck).GetSeconds ();
  // m_lastAck = currentAck;

  m_ackedSegments = 0;
  NS_LOG_LOGIC ("Estimated BW: " << m_currentBW);

  // Filter the BW sample not needed
  NS_LOG_LOGIC ("Estimated BW after filtering: " << m_currentBW);
}

void
TcpConstant::PktsAcked (Ptr<TcpSocketState> tcb, uint32_t packetsAcked, const Time& rtt)
{
  NS_LOG_FUNCTION (this << tcb << packetsAcked << rtt);

  if (rtt.IsZero ())
    {
      NS_LOG_WARN ("RTT measured is zero!");
      return;
    }

  m_ackedSegments += packetsAcked;
  EstimateBW(rtt, tcb);
}

void
TcpConstant::CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  uint32_t w = tcb->m_cWnd / tcb->m_segmentSize;

  // Floor w to 1 if w == 0 
  if (w == 0)
    {
      w = 1;
    }

  NS_LOG_DEBUG ("w in segments " << w << " m_cWndCnt " << m_cWndCnt << " segments acked " << segmentsAcked);
  if (m_cWndCnt >= w)
    {
      m_cWndCnt = 0;
      tcb->m_cWnd += tcb->m_segmentSize;
      NS_LOG_DEBUG ("Adding 1 segment to m_cWnd");
    }

  m_cWndCnt += segmentsAcked;
  NS_LOG_DEBUG ("Adding 1 segment to m_cWndCnt");
  if (m_cWndCnt >= w)
    {
      uint32_t delta = m_cWndCnt / w;

      m_cWndCnt -= delta * w;
      tcb->m_cWnd += delta * tcb->m_segmentSize;
      NS_LOG_DEBUG ("Subtracting delta * w from m_cWndCnt " << delta * w);
    }
  NS_LOG_DEBUG ("At end of CongestionAvoidance(), m_cWnd: " << tcb->m_cWnd << " m_cWndCnt: " << m_cWndCnt);
}

void
TcpConstant::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  // Linux tcp_in_slow_start() condition
  if (tcb->m_cWnd < tcb->m_ssThresh)
    {
      NS_LOG_DEBUG ("In slow start, m_cWnd " << tcb->m_cWnd << " m_ssThresh " << tcb->m_ssThresh);
      segmentsAcked = SlowStart (tcb, segmentsAcked);
    }
  else
    {
      NS_LOG_DEBUG ("In cong. avoidance, m_cWnd " << tcb->m_cWnd << " m_ssThresh " << tcb->m_ssThresh);
      CongestionAvoidance (tcb, segmentsAcked);
    }
}

std::string
TcpConstant::GetName () const
{
  return "TcpConstant";
}

uint32_t
TcpConstant::GetSsThresh (Ptr<const TcpSocketState> state,
                           uint32_t bytesInFlight)
{
  NS_LOG_FUNCTION (this << state << bytesInFlight);

  // In Linux, it is written as:  return max(tp->snd_cwnd >> 1U, 2U);
  return std::max<uint32_t> (2 * state->m_segmentSize, state->m_cWnd / 2);
}

Ptr<TcpCongestionOps>
TcpConstant::Fork ()
{
  return CopyObject<TcpConstant> (this);
}

} // namespace ns3

