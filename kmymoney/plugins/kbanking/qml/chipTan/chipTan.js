/*
 * A tan input dialog for optical chipTan used in online banking
 * Copyright 2014  Christian David <c.david@christian-david.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

var transmitCode = []

var currentIndex = 0
var currentIntervalStarted = true

function checkData(dataString)
{
  return true;
}

function timerStarted( data, parentItem )
{
  // Startcode
  transmitCode = [ 0xF, 0x0, 0xF, 0xF ]

  // Set Code (half-bytes are switched)
  for (var i = 0; i < data.length; i+=2) {
      transmitCode.push(parseInt(data[i+1], 16))
      transmitCode.push(parseInt(data[i], 16))
  }
}

function charToHex( char )
{
  switch(data[i]) {
  case '0': return 0x0
  case '1': return 0x1
  case '2': return 0x2
  case '3': return 0x3
  case '4': return 0x4
  case '5': return 0x5
  case '6': return 0x6
  case '7': return 0x7
  case '8': return 0x8
  case '9': return 0x9
  case 'A': return 0xA
  case 'B': return 0xB
  case 'C': return 0xC
  case 'D': return 0xD
  case 'E': return 0xE
  case 'F': return 0xF
  }
}

function timerTriggered( parentItem )
{
  var colorOn = parentItem.children[0].colorOn
  var colorOff = parentItem.children[0].colorOff

  if (currentIntervalStarted == true) {
      parentItem.children[0].color = colorOff
      currentIntervalStarted = false;
      return;
  }

  ++currentIndex
  currentIntervalStarted = true
  if (currentIndex >= transmitCode.length)
      currentIndex = 0

  parentItem.children[0].color = colorOn
  parentItem.children[1].color = (transmitCode[currentIndex] & 1) ? colorOn : colorOff
  parentItem.children[2].color = (transmitCode[currentIndex] & 2) ? colorOn : colorOff
  parentItem.children[3].color = (transmitCode[currentIndex] & 4) ? colorOn : colorOff
  parentItem.children[4].color = (transmitCode[currentIndex] & 8) ? colorOn : colorOff
}
