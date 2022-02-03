
#ifndef TCPCONSTANT_H
#define TCPCONSTANT_H

#include "ns3/tcp-congestion-ops.h"
#include "ns3/tcp-socket-state.h"

namespace ns3 {

class TcpConstant : public TcpCongestionOps
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  TcpConstant ();

  /**
   * \brief Copy constructor.
   * \param sock object to copy.
   */
  TcpConstant (const TcpConstant& sock);

  ~TcpConstant ();  

  std::string GetName () const;

  virtual void IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
  virtual uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb,
                                uint32_t bytesInFlight);

  virtual void PktsAcked (Ptr<TcpSocketState> tcb, uint32_t packetsAcked,
                          const Time& rtt);  

  virtual Ptr<TcpCongestionOps> Fork ();

protected:
  /**
   * Slow start phase handler
   * \param tcb Transmission Control Block of the connection
   * \param segmentsAcked count of segments acked
   * \return Number of segments acked minus the difference between the receiver and sender Cwnd
   */
  virtual uint32_t SlowStart (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
  /**
   * Congestion avoidance phase handler
   * \param tcb Transmission Control Block of the connection
   * \param segmentsAcked count of segments acked
   */
  virtual void CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
  
  TracedValue<double>    m_currentBW;              //!< Current value of the estimated BW
  uint32_t               m_ackedSegments;          //!< The number of segments ACKed between RTTs
  bool                   m_IsCount;                //!< Start keeping track of m_ackedSegments for Westwood+ if TRUE 

private:
  uint32_t m_cWndCnt {0}; //!< Linear increase counter
  uint32_t m_k; //slow start for first k rounds
  uint32_t m_beta; //rtt change threshold

  /**
   * Estimate the network's bandwidth
   *
   * \param [in] rtt the RTT estimation.
   * \param [in] tcb the socket state.
   */
  void EstimateBW (const Time& rtt, Ptr<TcpSocketState> tcb);
};

} // namespace ns3

#endif // TCPCONSTANT_H

