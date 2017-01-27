// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014-2015 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#include <vector>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <cmath>
#include <string>
#include <unistd.h>
#endif

#include "base64.h"
#include "browserlog.h"
#include "mongoose.h"
#include "webstatic.h"


static const char* boinc_png =
    "iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAAAXNSR0IArs4c6QAAAARnQU1BAA"
    "Cxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAAWdEVYdFNvZnR3YXJlAHBhaW50Lm5ldCA0"
    "LjA76PVpAAAP+klEQVR4Xu1bCVRURxZtYyaZZJzJTAQUHU2MmkzGmMXJHg9BRREwGtAxuKJG1G"
    "jiLogCQWURRBYR0QCiAqKAiiuyNLIJSKJAq6CAoOyyNmvTDXTNe7+rW5r+H6FBj5wz95x3oP9/"
    "r+rdV8uvqv8+7//oAkLIG0WPhZ7wdwi9NOAhlUqH5pZWuwCnV+gldoDC69v94xLHLz9Eom8+SA"
    "PDN+mtAQvgMMznQrpg5AIP4nU+PQw4vkxvKQOj4xBy/fLQefvJmyb7yAhTN3Iq7rYACtCmKgMO"
    "4PtIt9PJORomLgwnbVN34h+VGQRcB1MVGfCCZ8SNU5rz3RjFN8Fg6PfOZJiJM/G5cCMfChpDVQ"
    "cMgNNoh8D4PI05TgyXocayIGBPCI677Q/3B8kVX/K7mnlkxEJPJfJD0XC2I9GY40icguJLIAj/"
    "ZgwGAEQi0Vhbv+hC9J8RJgh7FUF4a/EBEp6U480EISwp23nUEq8n5I2fkGcC8J090Zi1h2zzvl"
    "IlkUg+pXW8sGhtlb5n4X25BH1G3xVBQOkUhLFmBwk/4+F+nqCgYrXe9qB2vIg3GaUu5DWMdhMN"
    "w13E3Cmsvq6xUZfW9cKhUSz+YKP7uXL0lfG5myDobw+SFlUKbRjDwnKh6WybEIkyeQcl8poGv4"
    "LYkvk7j7cUV9YbMYY9BHY1GEKa8FevWSRZm5Nf5shPzT58KV5w7GJ81tH49HsHCkqqrVvE4iWg"
    "NwnkVWraYzQ1iT9Z4xRahT6irxoGduA3+G4kD4KDgpvh9kBp0eO6bdRUhsr6JsMFe8JalMmDdC"
    "KvoW9DNGfsJAYbDkuy8ysWUFNW4NwCMvl+QbnbkbCEexucQqS6y2D86VmSYTqbWWXszB3EcI0n"
    "sdgfLvINS+Q/Kq3eCMH4Jy2SE0Jh8+fL7YJqNWdYy3zkDIIjmb0jqKO8uvEXaqqMhhaJzkqXc/"
    "VM16fksRCmMEpeczqI3g6is9KjI01QsJqaKgCkh9Q3Nm8IiLieP3fjIaL97RZWsj2RUdO2kRXW"
    "xzr4aTlnuXpF6eOGyQt3BNSjT4xv4KOGvjXRnKkahHk2we2VdU3m1JQdzRLJfzYeuFTFGMnJz7"
    "QhGhBdOXlNve1Ec6ol+XShkzQ6NdsS7YD4YGFD80/eIXFVH5vsYiWkjny9yEmSlJlvwjjXBcXl"
    "dVPmbf2tCX3RnAY+dRMEU9vg9tr6lqXUtHtAtN+38Y0q1jKErg+FMIVNB2HIWzHktaZaEK0p28"
    "gHxrsIjGOfywmCjG8W72Uloa58scBRzE/PZiV/r7DMYPYv3i3ogyb4ohQEPQwCDgdZEJbsOimp"
    "EbaYUtOeAYLwlktQXJ4WtD7T9aFQLUoeK8SKtb7dSrTAURzXw/vQ1dlk0rzdonP8W4bUHSU8LK"
    "1aZLTWs11LZ4vMB/QFfaJB0FIEYSdZtjtY3NDcakxNewfo1sN9ziRnaWNXwkKnyVveggzTRfL9"
    "S1ouH5nsEh2/mMpKHp4WZt+t9eqQ62IDKIIAfsmCAAL+mu8JaWlqbWMtp8eAIPzjxKX01FEG1k"
    "zhTCVqkn9HfzuZ88tBssUlVGJ/5FLF/oCo/P3Hogrtj1yu3uxyun2hhS9Zuyeo5ULsLQNavRKy"
    "88vM9M3dFeQ7i3IQLMgah5AmUVubHjXtG3BmP8PPiBljZCOrpBfkR+tZkFV2J1oDL6ZeEDaIfo"
    "KhNQHK+zMtWgG4/jeQD0D+Si8p4Vb2I7Mpy11ZyctFS0cWhHWOIQ2watWhpv0DcOzVq9fvnv3X"
    "7F+ZbsfmQGcZNc2CbN0XVnn3QdlOsO3Ttjo1M/+nyYuduyUvl/VOIXVA/itq2r/AR13CH3mBH5"
    "nYsVYul+kr3Trif79/EPT/Tk3VBpTxemxqTtRnP9iz1tVZZv/sRSQS6bfU9NkAHHopLavA9QtT"
    "BymbEyttjtU8rhHqU/V+Adb5oKhqi8nGQ+1sdcrln1O3kXP8DA9q9myRfrtgg85SF6Vuud7pZB"
    "F09/eoCiekyby/SmN5k8g1nh6J4elK43nj4X/2k5pOqK1vnr3I0q+1c51dBXpfGy7mqMmzhSC3"
    "aNGMlW4SrHjpdr9K3IrSWyoAgkOkcbx10mu8NCDcRuKhYTvLNZ5QyuedAplKeDzZYQULquoavp"
    "+19kC3PcH9eAyfqvcP2tvbcZf2Gv2phPziSoPNLqeE5VV1U+glFQBpUyBfQRKAaBJIMsh1lEFU"
    "6PVEEAgG6PLBZhw1V4Egr9hqwhxbVvIosCKVtrT000SIM7i1V4TofFxGAozFN+hlJYDOMPqvEk"
    "gobzCQOcS0MBJEsqmwSbwxmJB0lJdlgr/T4HoK3MfgQKDArh6EdS7BOcHvTFIyG3m5+IYnhFD1"
    "vqGwpGrN2zOsmAnGK5ifAWR7dEhKCG8QtKIf0+pICokj6T/+RMjNV1QFr/8OwZAHAnoDBEAMcw"
    "TrYgb8mDir04qwq5jt8G8Bnb9RdfXhGcSPkS98hoNs2ReaLxKJ3qG3OQHk1zAtj+SRFJJjI95V"
    "MBDYI+RBuMarhglzJC1WCQHnkkO7EpfLO/pWpEbY2O2ZxVOBz+B1jifFXVd+y60DyhrF4olUTQ"
    "XSSJ42ON7AdHts+Z6Sl4s8CDhkcDjweWG0aCWIRG26n87nXh/4hiUGUFX1AF3oqy8X7WVd+Xmc"
    "iMmG+6yHFOCwM9P1sRVxjLORfJpgELDnQA+CYHZAmRNo8QrgXLDZ+XR5V9/kYul2JoeqqodHZT"
    "WrhsPamq3wpVZ4tE5Uxied+B4zrY8EuMZ8TwSDh0GU9QJXWoUSDobEBbL5h2Jm5S/maqQeISrl"
    "rhPustgKX7DtNwKFf0lVFWAWODj28dGGkx4bsZ4KBg+HkGxCFNAqlHBD8GALm38oej/uRx/HUt"
    "Xe49j5FH+uAPy0G98wqa71wVEzRffv7djvKvJhAL2JeSKwrBbrGpq/xx0nm48fmexCHydT1d7D"
    "+3R8CFcA4GnQBoUrv2MDwHjdxixoUiEAfen+csHJEJ8k1yAIiTxNWo0C4IPuOIMdrD6Oh+vQA2"
    "ZQ1d7DO4Q7ABZu4ewB4D/fAIATnAF412hn3wJwLOK6H1cAfnY4id3rhR4CH8/dhQH4hqr2HleS"
    "7zpyBQCPtlgnwXjeJ89zEiwqq+GcBKebu6GP6r/ZLqqoWcn1GHzPyJpIJO1rqKoCzGPwGmx8nt"
    "NjsFbY7KKty35Et2zn0Vbopd1nhXQHiN4XXAshlOiUu6wrNOiqe/t1IRTHvhBCgI9ToTeyHs5Y"
    "uoXfpWrqAQp/ba3DyVauQ1DbgxFNoKNyiAkBGP48lsJyJP6ea/k5ywmV35lkP6qiPjwCY69yBW"
    "CisR1pbBSxvmiEVlvdL5uhOF4V12aoM5Ju5S2dtsKVOZxBGTvTilTWNP5Ab6uPguJK89HTt6uQ"
    "l4tXcGwx9hSqrgCzHY7j+TJDAYOgznb4Gq8VyphGi3wqCstqZsIKtQn9MrM62szWO3sNmET+sd"
    "L2RDPXPPDBnF/Jo5LqvVRdCcyEyOcdVOdAhMQxR2Ssz3Dw6SUgx9ormpsln+FxvG94YhC91Hec"
    "ibnlM6yblyHmdsfbwCHO42hoxR9AyhhiGAgkyXEkBq0uBd0YIM+5hs+4V2y5ae/p0traxg/pJS"
    "WAL+NBWO+pBShszH83H1aMLzbxDIp9jBVTExWQC7zXgdg6kBQQlUNRuFYLEgITaLcpOFW19d8Z"
    "0UNR0y1HhOVVzyll51JClusIjjUBCj6Lj55NfgRBeJeacEIaxfsLs2uMocfi0bzxMGe8RG9zor"
    "K2ftZCC1+lY3Hj9d6tj0pq+j7ZPQ0w7oZsdg172LnyroJBcD8RU9PW1jadmvULoO5B+cWVG002"
    "sL8YMVjtIb1fUL6eqj87CHJLfhs5lX3d3VkwraW8Wsieh9NLQI8aF8HPiHzaqzGdpc7kZs6j3d"
    "Ss/yHIK93zjdk+RVIEmxNyWW4dcBscV9m59QZgPzYlM//g6l0nxD1JvsC1yqemDiT5Vp47PiVo"
    "MX0Hdj8g7/K1meuTzJBungg/2wffbmyUalFzJQCpt0DegzK5Xo9//rCkaqvPqfgkmHQ7epp1gv"
    "4wOQvg24S5e0h0anYw1KH+HkAOjKQgr8zjy2VustSTbhIk8Mh8m2voHSDBdmjx58ik20ErbY/D"
    "BiWAWLmfbbc/crHcxT/yvsvRyDz7I5eqN+49JdVf5U5GTGHffXJJZ/KYGIEpPOPmYN6SIArqVT"
    "/lH8ln5ZX6fI7k5dlhHCky2FJAIhfIq+T0IfkLcVmRY/S5V5PqClM/Qx6J00QpzGMCX0fPsiMh"
    "0TfT2RrkqQCnB2fllR39bIWHLNOqmySp4eAAtGIOVDSCmiuA5C8nCCLxJQUbAXVl5FR4HONcpC"
    "tLielKXpYqZ020jeyI3/m0++Bbz88EwOmXs/LLgyYt82Ry7LpLk9OGHuEZyM8AGw1qrgCSj03N"
    "juQ6rlJXZq72kFxJFHjsOxadoa0nG5Zs5OUZo1qGdsTtVGKpuCcrQ3D6lcy88tBPVhyQZVd2ky"
    "g5YoYVORSagF1MJf0Frr0Wn54TiWdybCTUEXjMSWHjFSUWi5lzAfD1Dd+z1+NGGwJRxifwDfxD"
    "P9FfpvHQd0z1NdpD7PxjMHWGO3sEnH41I78s4sMVB5m8WvZUWVkQRkIF/hEpKWDD+vIRnJscdC"
    "n19ux1XmQEDBE2Qj0RPOtbYukn9j+TGC6RSr+mxSuAPofzM8PeNbEHv2SJkV3JMzyYrHEHssnr"
    "sqi5lSVnEFssI6/8ykQg/7Rk6VEwuQRe+R1flXc7w8J9zBCfdDe3xNEjMPbWOvvgNiZZejp3sv"
    "R4w51MsvRG5xCh98m4i8UVNauhHJXh1Rlwf3DsjfveHy/ep0iN1WTyg5XJyzPFVzifaxc2iFZR"
    "c4b8X27llsVO+PFQl28FVNPl3zK2J6djM2Kg0tepeY+BAYO6vhGL21fcyS2xjUy+4x4e/cfhs7"
    "E3va9ev+NSAM9/uD8P5H3Q7fVCJv3OQ9vJq72ksuRoGfmhXcgzH4IYO5P/7g4llbVNNlDPIF5G"
    "/uPo93/0Yb6iUP1g4kkQxszbS84n3rkCRiqLmBcFOY8qVs3c7NfO1vKyz2acma9ikOt3tqdIpb"
    "B5Hy+7sMJYZ2ugpLtPZsaZupIrKTkRQL7vq6tnjKKKOuP5tidFbC0vJ//mvP1kqmWQOLdUKNu0"
    "CQoq53+96VibIgidPpp6d7EH4d/MDwXyT83ielHQ0NKiY+5yto4hz3wio0z+q03HO+4UVionT6"
    "TdK1vy5fqAjs5B+PdyLxKfWYhvQQcMeTngcfnhVp+rpbKPpJ6Q/2SdnzTtXvEmqqaMuMyHP/5n"
    "nR8ThImrfEhqdjEmAfTfzuo5A3x/2yE44b7GXFeG/PurjpCkO8Uu9DY74rMK187dHSbNeFDuM5"
    "DJywEcNA5funnjX+aHSXRG4XH4zZmDqAAo6fZIcYAAuAypqm+2g78Dbig/Y/B4/wMa6F2dR046"
    "ugAAAABJRU5ErkJggg==";


int handle_static_boinc_png(struct mg_connection *conn) {
    std::string image;
    image = r_base64_decode(boinc_png, strlen(boinc_png));

    mg_send_status(conn, 200);
    mg_send_header(conn, "Content-Type", "image/png");
    mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, pre-check=0, no-store, no-cache, must-revalidate");
    mg_send_header(conn, "Access-Control-Allow-Origin", "*");
    mg_send_data(
        conn,
        image.c_str(),
        (int)image.size()
    );
    return MG_TRUE;
}

