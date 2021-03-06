### Description
![Check valve with orifice picture](checkvalveorifice_user.svg)

Hydraulic check valve with orifice in reverse direction using a turbulent flow model.

#### Input Variables
* **K_s** - Restrictor coefficient [-]
* **K_r** - Restrictor coefficient in opposite direction [-]

#### Port Initial Conditions
No initial conditions can be set for Q-type blocks.

<!--- ### Tips--->

### Theory
The component is modelled as a turbulent orifice with different restrictor coefficients in different directions. The turbulent flow through the orifice is proportional to the square root of the pressure difference according to equation EQREF{eq:flow}
<!---EQUATION LABEL=eq:flow q_{2} =\begin{cases}K_s \sqrt{p_{1}-p_{2}}, & p_1 > p_2 \\K_r \sqrt{p_2-p_1}, & p_2 \le p_1\end{cases} --->

#### Hopsan TLM adaption
In Q components the positive flow direction is outwards from each port, in this case the TLM equations are
<!---EQUATION p_{1} = c_{1} + q_{1} Z_{c1} --->
<!---EQUATION p_{2} = c_{2} + q_{2} Z_{c2} --->
<!---EQUATION q_{1} = -q_{2} --->
Inserting this into equation EQREF{eq:flow} and taking negative pressure drops into account gives the implemented equation EQREF{eq:final}:

<!---EQUATION LABEL=eq:final q_2 = \begin{cases} K_s \left(\sqrt{c_1-c_2+\dfrac{(Z_{c1}+Z_{c2})^2K_s^2}{4}} - K_s\dfrac{Z_{c1}+Z_{c2}}{2}\right), & c_1 > c_2\\ K_r \left(\sqrt{c_1-c_2+\dfrac{(Z_{c1}+Z_{c2})^2K_s^2}{4}} - K_s\dfrac{Z_{c1}+Z_{c2}}{2}\right), & c_1 \le c_2 \end{cases} --->

